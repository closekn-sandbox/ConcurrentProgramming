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
#include <pthread.h>

#define MAXPROC 100
#define genrnd(mn, mx) (rand() % ((mx)-(mn)+1) + (mn))

// ring buffer
struct ringbuf {
  int bufsize;  // The size of ring buffer. If minus values indicates illegal state.
  int wptr, rptr;
  int n_item;   // The number of items in ring buffer
  int buf[1];   // For variable array size
} *rbuf;

// prodとconsの構造体
typedef struct {
  int no;
  int num;
} Agent;

// mutex_t
pthread_mutex_t lock_status;

//-- prod cons
void producer(Agent *a);
void consumer(Agent *a);

int main(int argc, char *argv[])
{
  int N;    // リングバッファサイズ
  int P;    // プロデューサプロセス数
  int C;    // コンシューマプロセス数
  int L;    // 1以上の整数

  int n_prod, n_cons;
  int pid;
  int status;
  int pnum, nid;
  int pmap[MAXPROC], cmap[MAXPROC];
  // add varables
  pthread_t *prod_threads;
  pthread_t *cons_threads;
  Agent *prod_agent;
  Agent *cons_agent;
  struct timespec t_s, t_e;
  struct timespec c;
  long double s, ns, cs;
  struct ringbuf rbuf_base[1];

  // 引数不足
  if (argc < 5) {
    fprintf(stderr, "Usage: %s N n\n", argv[0]);
    fprintf(stderr, "N: size of ring buffer\n"
      "n: The number of production and consumption numbers\n");
    exit(1);
  }
  N = atoi(argv[1]);
  P = atoi(argv[2]);
  C = atoi(argv[3]);
  L = atoi(argv[4]);
  // 不適なパラメタ
  if (N < 15 || P <= 0 || C <= 0 || L <= 0 || P*C*L < 3*N ) {
    fprintf(stderr, "Parameter error\n");
    exit(2);
  }

  // malloc threads
  prod_threads = malloc(sizeof(pthread_t) * P);
  cons_threads = malloc(sizeof(pthread_t) * C);
  if(prod_threads == NULL || cons_threads == NULL) {
    perror("malloc threads");
    exit(1);
  }

  prod_agent = malloc(sizeof(Agent) * P);
  cons_agent = malloc(sizeof(Agent) * C);
  if(prod_agent == NULL || cons_agent == NULL) {
    perror("malloc threads");
    exit(1);
  }

  rbuf = rbuf_base;
  rbuf->bufsize = N;
  rbuf->n_item = 0;
  rbuf->wptr = rbuf->rptr = 0;

  pthread_mutex_init(&lock_status, NULL);

  clock_gettime(CLOCK_REALTIME, &t_s);

  // create consumer
  n_cons = 0;
  while(n_cons < C) {
    cons_agent[n_cons].no = n_cons;
    cons_agent[n_cons].num = L;
    pthread_create(&cons_threads[n_cons], NULL, (void *)consumer, &cons_agent[n_cons]);
    printf("cons Thread %d is created\n", n_cons);
    n_cons++;
  }

  // create producer
  n_prod = 0;
  while(n_prod < P) {
    prod_agent[n_prod].no = n_prod;
    prod_agent[n_prod].num = L;
    pthread_create(&prod_threads[n_prod], NULL, (void *)producer, &prod_agent[n_prod]);
    printf("prod Thread %d is created\n", n_prod);
    n_prod++;
  }
  // join threads
  for (pnum = 0; pnum < n_prod; pnum++) {
    pthread_join(prod_threads[pnum], (void **)&status);
  }
  for (pnum = 0; pnum < n_cons; pnum++) {
    pthread_join(cons_threads[pnum], (void **)&status);
  }

  clock_gettime(CLOCK_REALTIME, &t_e);
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &c);
  s = (t_e.tv_sec - t_s.tv_sec);
  ns = (long double)(t_e.tv_nsec - t_s.tv_nsec);
  s += ns * 10e-10;
  cs = c.tv_sec + c.tv_nsec * 10e-10;
  printf("%Lf %Lf\n", s, cs);
  // free memories
  free(prod_threads);
  free(cons_threads);
  free(prod_agent);
  free(cons_agent);
  return 0;
}

// prod
void producer(Agent *a)
{
  int rnd;
  int prod_No = a->no;
  int pnum = a->num;

  printf("I am producer process.\n");
  srand(time(NULL) ^ (prod_No << 8));
  for (; pnum; pnum--) {
    rnd = genrnd(20,80);
    // put random number into ring buffer
    while (1) {
      pthread_mutex_lock(&lock_status);
      if (rbuf->bufsize > rbuf->n_item) { break; }
      // illegal state occurs and operation stops
      if (rbuf->bufsize < 0) {
        pthread_mutex_unlock(&lock_status);
        return;
      }
      // reduce waste of CPU resource
      usleep(1);
      pthread_mutex_unlock(&lock_status);
    }
    rbuf->buf[rbuf->wptr++] = rnd;
    rbuf->wptr %= rbuf->bufsize;
    rbuf->n_item++;
    printf("P#%02d puts %2d, #item is %3d\n", prod_No, rnd, rbuf->n_item);
    fflush(stdout);
    pthread_mutex_unlock(&lock_status);
    rnd = genrnd(20,80);
    usleep(rnd*1000);
  }
  pthread_exit(0);
}

// cons
void consumer(Agent *a)
{
  int rnd;
  int cons_No = a->no;
  int pnum = a->num;

  printf("I am consuer process.\n");
  for (; pnum; pnum--) {
    // pick number from ring buffer
    while (1) {
      pthread_mutex_lock(&lock_status);
      if (rbuf->n_item) { break; }
      // illegal state and stop operation
      if (rbuf->bufsize < 0) {
        pthread_mutex_unlock(&lock_status);
        return;
      }
      usleep(1); // reduce waste of CPU
      pthread_mutex_unlock(&lock_status);
    }
    rnd = rbuf->buf[rbuf->rptr++];
    rbuf->rptr %= rbuf->bufsize;
    rbuf->n_item--;
    printf("C#%02d gets %d, #item is %3d\n", cons_No, rnd, rbuf->n_item);
    fflush(stdout);
    pthread_mutex_unlock(&lock_status);
    usleep(rnd*1000);
  }
  pthread_exit(0);
}
