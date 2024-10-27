#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
char*
fmtname(char *path)
{
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;
  return p;
}

void find(char *path, char *file_name){
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;
  if((fd = open(path, O_RDONLY)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  } 
  if(fstat(fd, &st) < 0){
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }
  //printf("fmtname:%s,find:%s\n",fmtname(path),fmtname(file_name));
  if(strcmp(fmtname(path),fmtname(file_name))==0){
    printf("%s\n",path);
  }
  switch(st.type){
  case T_DEVICE:
  case T_FILE:
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("find: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(strcmp(de.name,".")==0){
        continue;
      }
      if(strcmp(de.name,"..")==0){
        continue;
      }
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      //printf("%s\n",buf);
      //sleep(10);
      find(buf,file_name);
    }
    break;
  }
  close(fd);
}

int main(int argc,char *argv[]){
  if(argc<3){
    exit(1);
  }
  find(argv[1],argv[2]);
}