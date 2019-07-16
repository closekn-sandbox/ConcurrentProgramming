#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <time.h>

#define MAXPROC 100
#define genrnd(mn, mx) (rand() % ((mx)-(mn)+1) + (mn))

// リングバッファ
struct ringbuf {
  int bufsize;    // リングバッファのサイズ
  int wptr, rptr;
  int n_item;     // リングバッファ格納数
  int buf[1];     // 可変配列サイズ
} *rbuf;

// セマフォと共有メモリ
int semid;
int shmid;

//-- ope
void P();
void V();
//-- process
void producer(int prod_No, int pnum);
void consumer(int cons_No, int cnum);
// 共有メモリとセマフォのリリース
void release();

int main(int argc, char *argv[])
{
  int N;    // リングバッファサイズ
  int l;    // プロデューサプロセス数
  int L;    // プロデューサプロセスサイズ
  int m;    // コンシューマプロセス数
  int M;    // コンシューマプロセスサイズ
  int n_prod, n_cons;
  int pid;
  int status;
  int pnum, nid;
  int pmap[MAXPROC], cmap[MAXPROC];
  struct timespec t_s, t_e, c;
  long double s, ns, cs;

  // 引数不足
  if (argc < 6) {
    fprintf(stderr, "Usage: %s N n\n", argv[0]);
    fprintf(stderr, "N: size of ring buffer\n"
            "n: The number of production and consumption numbers\n");
    exit(1);
  }
  N = atoi(argv[1]);
  l = atoi(argv[2]);
  L = atoi(argv[3]);
  m = atoi(argv[4]);
  M = atoi(argv[5]);
  if (N <= 0 || l <= 0 || L <= 0 || m <= 0 || M <= 0 || l * L != m * M) {
    fprintf(stderr, "Parameter error\n");
    exit(2);
  }
  shmid = shmget(IPC_PRIVATE, sizeof(struct ringbuf) + (N-1)*sizeof(int), IPC_CREAT | 0666);
  if (shmid == -1) {
    perror("shmget");
    exit(1);
  }
  rbuf = (struct ringbuf *)shmat(shmid, NULL, 0);
  if (rbuf == (struct ringbuf *)-1) {
    perror("shmat");
    exit(1);
  }
  rbuf->bufsize = N;
  rbuf->n_item = 0;
  rbuf->wptr = rbuf->rptr = 0;
  semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
  if (semid == -1) {
    perror("semget");
    if (shmctl(shmid, IPC_RMID, NULL)) { // release shared memory area
        perror("shmctl");
    }
    exit(1);
  }
  if (semctl(semid, 0, SETVAL, 1) == -1) {
    perror("semctl at initializing value of semaphore");
    release();
    exit(1);
  }

  clock_gettime(CLOCK_REALTIME, &t_s);
  // create consumer
  n_cons = 0;
  while(n_cons < M) {
    pid = fork();
    switch (pid) {
      case 0:
        consumer(n_cons, m);
        exit(0);
        break;
      case -1:
        printf("Fork error\n");
        release();
        exit(1);
      default:
#ifdef WITH_NOT_DEBUG
#else
        printf("Process id of consumer process %d is %d\n", n_cons, pid);
#endif
        cmap[n_cons] = pid;
        n_cons++;
    }
  }

  // create producer
  n_prod = 0;
  while(n_prod < L) {
    pid = fork();
    switch (pid) {
      case    0:
        producer(n_prod, l);
        exit(0);
        break;
      case    -1:
        printf("Fork error\n");
        rbuf->bufsize = -1;  // stop consumer
        for (pnum = 0; pnum < n_cons; pnum++) {
          pid = wait(&status); // wait for termination of consumer process
        }
        release();
        exit(1);
      default:
#ifdef WITH_NOT_DEBUG
#else
        printf("Process id of producer process %d is %d\n", n_prod, pid);
#endif
        pmap[n_prod] = pid;
        n_prod++;
    }
  }
  // main process only reaches this position
  for (pnum = n_cons+n_prod; pnum > 0; pnum--) {
    pid = wait(&status);
    for (nid = 0; nid < n_cons; nid++) {
      if (pid == cmap[nid]) {
#ifdef WITH_NOT_DEBUG
#else
        printf("Consumer %d finished at %ld\n", nid, time(NULL));
#endif
        break;
      }
    }
    if (nid != n_cons) { continue; }
    for (nid = 0; nid < n_prod; nid++) {
      if (pid == pmap[nid]) {
#ifdef WITH_NOT_DEBUG
#else
        printf("Producer %d finished at %ld\n", nid, time(NULL));
#endif
        break;
      }
    }
    if (nid != n_prod) { continue; }
      printf("Illegal process ID %d\n", pid);
  }
  release();
  clock_gettime(CLOCK_REALTIME, &t_e);
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &c);
  s = (t_e.tv_sec - t_s.tv_sec);
  ns = (long double)(t_e.tv_nsec - t_s.tv_nsec);
  s += ns * 10e-10;
  cs = c.tv_sec + c.tv_nsec * 10e-10;
  printf("%Lf,%Lf\n", s, cs);

  return 0;
}

//-- operation --
void P()
{
  struct sembuf sops;

  sops.sem_num = 0;        // semaphore number
  sops.sem_op = -1;        // operation (decrement semaphore)
  sops.sem_flg = SEM_UNDO; // The operarion is canceled when the calling porcess terminates
  semop(semid, &sops, 1);  // Omitt error handling
}

void V()
{
  struct sembuf sops;

  sops.sem_num = 0;        // semaphore number
  sops.sem_op = 1;         // operation (increment semaphore)
  sops.sem_flg = SEM_UNDO; // The operarion is canceled when the calling porcess terminates
  semop(semid, &sops, 1);  // Omitt error handling
}
//-- --

//-- process --
void producer(int prod_No, int pnum)
{
  int rnd;

#ifdef WITH_NOT_DEBUG
#else
  printf("I am producer process.\n");
#endif
  srand(time(NULL) ^ (prod_No << 8));
  for (; pnum; pnum--) {
    rnd = genrnd(20,80);
    // put random number into ring buffer
    while (1) {
      P();
      if (rbuf->bufsize > rbuf->n_item) { break; }
      // illegal state occurs and operation stops
      if (rbuf->bufsize < 0) {
        V();
        return;
      }
      // reduce waste of CPU resource
      usleep(1);
      V();
    }
    rbuf->buf[rbuf->wptr++] = rnd;
    rbuf->wptr %= rbuf->bufsize;
    rbuf->n_item++;
#ifdef WITH_NOT_DEBUG
    printf("%d\n", rbuf->n_item);
#else
    printf("P#%02d puts %2d, #item is %3d\n", prod_No, rnd, rbuf->n_item);
#endif
    fflush(stdout);
    V();
    rnd = genrnd(20,80);
    usleep(rnd*1000);
  }
}

void consumer(int cons_No, int cnum)
{
  int rnd;
#ifdef WITH_NOT_DEBUG
#else
  printf("I am consuer process.\n");
#endif
  for (; cnum; cnum--) {
    // pick number from ring buffer
    while (1) {
      P();
      if (rbuf->n_item) { break; }
      // illegal state and stop operation
      if (rbuf->bufsize < 0) {
        V();
        return;
      }
      usleep(1); // reduce waste of CPU
      V();
    }
    rnd = rbuf->buf[rbuf->rptr++];
    rbuf->rptr %= rbuf->bufsize;
    rbuf->n_item--;
#ifdef WITH_NOT_DEBUG
    printf("%d\n", rbuf->n_item);
#else
    printf("C#%02d gets %d, #item is %3d\n", cons_No, rnd, rbuf->n_item);
#endif
    fflush(stdout);
    V();
    usleep(rnd*1000);
  }
}
//-- --

// release share memory and semaphore
void release()
{
  if (shmctl(shmid, IPC_RMID, NULL)) {
    perror("shmctl");
  }
  if (semctl(semid, 0, IPC_RMID)) {
    perror("semctl");
  }
}
