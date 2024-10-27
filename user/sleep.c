#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if(argc != 2){
    fprintf(2,"sleep:wrong number of arguments, sleep only take one integer argument!\n");
  }
  int ticks = atoi(argv[1]);
  sleep(ticks);
  exit(0);
}
