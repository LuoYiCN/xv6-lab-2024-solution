#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int p2c[2],c2p[2];
  pipe(p2c);
  pipe(c2p);
  char bt;
  if(fork()==0){
    close(p2c[1]);
    close(c2p[0]);
    read(p2c[0],&bt,1);
    printf("%d: received ping\n",getpid());
    write(c2p[1],&bt,1);
  }
  else{
    close(c2p[1]);
    close(p2c[0]);
    write(p2c[1],&bt,1);
    read(c2p[0],&bt,1);
    printf("%d: received pong\n",getpid());
  }
  exit(0);
}