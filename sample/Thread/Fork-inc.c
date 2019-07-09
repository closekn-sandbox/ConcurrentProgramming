#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define LOOP 5

volatile int x;

int main()
{
  int pid;
  int status;
  int i;
  
  if ((pid = fork()) == -1) {
    printf("Fork error\n");
    exit(1);
  }
  if (pid) {
    printf("Process id of child process is %d,  x's addres is %p\n", pid, &x);
  } else {
    printf("I am child process, x's addres is %p\n", &x);
  }
  for (i = 0; i < LOOP; i++) {
    x++;  printf("%s: x = %d\n", (pid) ? "Parent" : "Child ", x);
    sleep(1);
  }
  if (pid) {  // Parent prrocess
    wait(&status);
    printf("Return value from child process is %d\n", status);
  }
  return 0;
}
