#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"


int
main(int argc, char *argv[])
{
  int tc=0,fp=0,p[2];
  pipe(p);
  if(fork()==0){
    close(p[1]);
    fp = p[0];
    goto child;
  }
  else{
    close(p[0]);
    tc = p[1];
    for(int i=2;i<=280;i++){
        write(tc,&i,sizeof(i));
    }
    close(tc);
    wait(0);
    //printf("%d:exiting\n",getpid());
    exit(0);
  }
child://first child begins here
  int buf,self,forked =0;
  while(read(fp,&buf,sizeof(buf))>0){
    if(forked){
        if(buf%self!=0){
            write(tc,&buf,sizeof(buf));
        }
    }
    else{
        printf("prime %d\n",buf);
        self = buf;
        pipe(p);
        if(fork()==0){
            close(p[1]);
            fp = p[0];
            forked = 0;
        }
        else{
            close(p[0]);
            tc = p[1];
            forked = 1;
        }
    }
  }
  if(forked){
    close(tc);
  }
  wait(0);
  exit(0);
}