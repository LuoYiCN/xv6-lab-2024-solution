// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
  char fill[PGSIZE-sizeof(struct run*)];
};

struct {
  struct spinlock lock;
  struct run *rest_page_first;
  int pageUsedinSuper[64];
  struct run *(first[64]);
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  acqure(&kmem.lock);
  for(int i=0;i<64;i++){
    kmem.pageUsedinSuper[i] = 512;
  }
  release(&kmem.lock);
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  char *superstart = (char*)SUPERPGROUNDUP((uint64)pa_start);
  for(;p+PGSIZE<=superstart;p+=PGSIZE){
    kfree(p);
  }
  for(; p + SUPERPGSIZE <= (char*)pa_end; p += SUPERPGSIZE){
    superfree(p);
  }
  for(; p+PGSIZE <= (char*)pa_end;p+=PGSIZE){
    kfree(p);
  }
}

#define PA2IND(pa) ((SUPERPGROUNDDOWN((uint64)pa) - SUPERPGROUNDUP((uint64)end))/SUPERPGSIZE)

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)((uint64)pa);
  if(r<SUPERPGROUNDUP((uint64)end)){
    acquire(&kmem.lock);
    r->next = kmem.rest_page_first;
    kmem.rest_page_first = r;
    release(&kmem.lock);
    return;
  }

  int ind = PA2IND(r);

  acquire(&kmem.lock);
  r->next = kmem.first[ind];
  kmem.first[ind] = r;
  --kmem.pageUsedinSuper[ind];
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.rest_page_first;
  if(r){
    kmem.rest_page_first = r->next;
  }
  else{
    int maxind = -1;
    for(int i=0;i<64;i++){
      if(kmem.pageUsedinSuper[i] == 512){
        continue;
      }
      int firstflag = (maxind == -1);
      int used = kmem.pageUsedinSuper[i];
      if(firstflag){
        maxind = i;
      }
      else{
        if(used > kmem.pageUsedinSuper[maxind]){
          maxind = i;
        }
      }
      r = kmem.first[maxind];
      kmem.first[maxind]=r->next;
    }
    
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

void *
superalloc(void)
{
  struct run *r;
  acquire(&kmem.lock);
  for(int i=0;i<64;i++){
    if(kmem.pageUsedinSuper[i] == 0){
      r = SUPERPGROUNDDOWN((uint64)kmem.first[i]);
      kmem.pageUsedinSuper[i] = 512;
      release(&kmem.lock);
      return (void*)r;
    }
  }
  release(&kmem.lock);
  return (void*)0;
}

void 
superfree(void* pa){
  int ind = PA2IND(pa);
  acquire(&kmem.lock);
  kmem.pageUsedinSuper[ind] = 0;
  kmem.first[ind] = 0;
  struct run *r = (struct run *)SUPERPGROUNDDOWN((uint64)pa);
  for(int i=0;i<512;i++){
    r[i].next = kmem.first[ind];
    kmem.first[ind] = (r+i);
  }
  release(&kmem.lock);
}