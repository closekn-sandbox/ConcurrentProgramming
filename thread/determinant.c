#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

// Matrixes
int *A, *B, *C;
// the number of threads
int n;

// 行列を一時配列で保管するため
typedef struct {
  int A_rows;
  int B_columns;
} Matrix_elem;

void multi_mat_elem(Matrix_elem *m);  // calc

int main(int argc, char *argv[]) {
  int i, j, k = 0;
  int pos;
  int threadnum;
  int *status;
  Matrix_elem *m;
  pthread_t *threads;
  struct timespec t_s, t_e;
  long double s, ns;
  struct timespec c;
  long double cs;

  if (argc < 3) {
    fprintf(stderr, "argv[1] = size of matrixes, argv[2] = the number of thread");
    exit(1);
  }

  n = atoi(argv[1]);
  threadnum = atoi(argv[2]);

  // 領域確保
  m = malloc(sizeof(Matrix_elem) * n * n);
  A = malloc(sizeof(int) * n * n);
  B = malloc(sizeof(int) * n * n);
  C = malloc(sizeof(int) * n * n);

  if (m == NULL || A == NULL || B == NULL) {
    fprintf(stderr, "don't ensure memories");
    exit(2);
  }

  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      pos = i * n + j;
      A[pos] = rand() % 10 + 1;
      B[pos] = rand() % 10 + 1;
      C[pos] = 0;
      m[pos].A_rows = i;
      m[pos].B_columns = j;
    }
  }

  // calc
  clock_gettime(CLOCK_REALTIME, &t_s);
  threads = malloc(sizeof(pthread_t) * threadnum);
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      pthread_create(&threads[k], NULL, (void *)multi_mat_elem, &m[i * n + j]);
      if (++k == threadnum) {
        for (k = 0; k < threadnum; k++) {
          pthread_join(threads[k], (void **)&status);
        }
        k = 0;
      }
    }
  }
  clock_gettime(CLOCK_REALTIME, &t_e);
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &c);
  s = (t_e.tv_sec - t_s.tv_sec);
  ns = (long double)(t_e.tv_nsec - t_s.tv_nsec);
  s += ns * 10e-10;
  cs = c.tv_sec + c.tv_nsec * 10e-10;
  // real cpu time
  printf("%d %Lf %Lf\n", threadnum, s, cs);

  // リリース
  free(A);
  free(B);
  free(m);
  free(threads);
}

// C = A * B
void multi_mat_elem(Matrix_elem *m) {
  int i;
  int sum = 0;
  // Cij += Aik * Bkj
  for (i = 0; i < n; i++){
    C[m->A_rows * n + m->B_columns] += A[m->A_rows * n + i] * B[i * n + m->B_columns];
  }
  pthread_exit(m);
}