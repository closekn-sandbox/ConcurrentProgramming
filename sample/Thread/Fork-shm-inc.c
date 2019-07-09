#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>

#define LOOP 5

int main()
{
  int pid;
  int status;
  int i;
  int shmid;
  int *x;
  
  shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
  if (shmid == -1) {
    perror("shmget");
    exit(1);
  }
  if ((pid = fork()) == -1) {
    printf("Fork error\n");
    shmctl(shmid, IPC_RMID, NULL);
    exit(1);
  }
  if (pid) {
    printf("Process id of child process is %d\n", pid);
  } else {
    printf("I am child process\n");
  }
  x = (int *)shmat(shmid, NULL, 0);
  if (x == (int *)-1) {
    perror("shmat");
    shmctl(shmid, IPC_RMID, NULL);  // アタッチに失敗すれば解放
    exit(1);
  }
  printf("%s: allocated address is %p\n", (pid) ? "Parent" : "Child ", x);
  for (i = 0; i < LOOP; i++) {
    (*x)++;  printf("%s: x = %d\n", (pid) ? "Parent" : "Child ", *x);
    sleep(1);
  }
  if (pid) {
    wait(&status);
    printf("Return value from child process is %d\n", status);
    shmctl(shmid, IPC_RMID, NULL);  // 共有メモリ領域を解放
  }
  return 0;
}
