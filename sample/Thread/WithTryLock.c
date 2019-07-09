// ロックのテスト3
//   クリティカルリージョンに入るためのロックを trylock で行うことで
//   ブロックさせない (ただし，ロックが取れたかどうかの検査が必要)
 
 #include <pthread.h>
 #include <stdio.h>
 #include <unistd.h>
 #include <stdlib.h>
 
 #define LOOPNUM 10
 #define THNUM 5
 
pthread_mutex_t lockval;
 
void enter(int *thnum)
{
  int loop;
  int lw = 0;
 
  for (loop = 0; loop < LOOPNUM; loop++) {
    while (pthread_mutex_trylock(&lockval)) {
      if (!lw) printf("%d (loop = %d): Wait for lock\n", *thnum, loop); // ロック取れず
      lw = 1;
      usleep(1);  // CPU を浪費しないために 1 quantum 程度の間隔でロックの検査
    }
    lw = 0;
    // ロックは取れている
    printf("%d (loop = %d): Enter Critical Region\n", *thnum, loop);
    usleep(rand() % 100000+100000);  // クリティカルリージョンの代わり
    printf("%d (loop = %d): Exit Critical Region\n", *thnum, loop);
    pthread_mutex_unlock(&lockval); // クリティカルリージョンから出るときにアンロック
    usleep(rand() % 500000+500000);
  }
  pthread_exit(NULL);
}
 
int main()
{
  int i;
  pthread_t th[THNUM];
  int params[THNUM];
 
  pthread_mutex_init(&lockval, NULL);
  for(i = 0 ; i < THNUM ; i++) {
    params[i] = i;
    pthread_create(&th[i], NULL, (void (*))enter, &params[i]);
  }
  for(i = 0 ; i < THNUM ; i++) {
    pthread_join(th[i], NULL);
  }
  pthread_exit(NULL);
}
