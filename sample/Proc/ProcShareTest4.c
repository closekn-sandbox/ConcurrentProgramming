#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define CHILD_NUM 4
#define LOOPNUM 10

int main()
{
  int pid;
  int status;
  int i;
  int shmid;
  int *x;
  int child_num;
  int wchnum;

  shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);  // 共有変数領域の確保
  if (shmid == -1) {
    perror("shmget");
    exit(1);
  }
  x = (int *)shmat(shmid, NULL, 0);	// 確保した共有変数領域を仮想メモリ空間に対応させる
  if (x == (int *)-1) {
    perror("shmat");
    exit(1);
  }
  printf("Allocated address is %p\n", x);
  *x = 100;

  for (child_num = 0; child_num < CHILD_NUM; child_num++) {
    pid = fork();
    if (pid == 0) {
      printf("I am child process %d\n", child_num);
      break;
    }
    if (pid == -1) {
      printf("Fork error\n");
      break;
    }
    printf("Process id of child process is %d\n",pid);
  }
 
  for (i = 0; i < LOOPNUM; i++) {
    (*x)++;
    if (pid == 0) {
      printf("Child (%d): x = %d\n", child_num, *x);
    } else {
      printf("Parent: x = %d\n", *x);
    }
    usleep(((pid ? 1 : 0) + 1)*500000);
  }
  if (pid != 0) {
    for (wchnum = 0; wchnum < child_num; wchnum++) {
      if (wait(&status) == -1) {
	perror("Wait error\n");
      } else {
	printf("Return value is %d\n",status);
      }
    }
    // 共有メモリ領域の開放
    if (shmctl(shmid, IPC_RMID, NULL)) {
      perror("shmctl");
    }
  }
  return 0;
}
