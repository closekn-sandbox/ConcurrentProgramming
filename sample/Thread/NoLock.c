// ロックのテスト1
//   排他制御しないでクリティカルリージョンに入れるようにしてみた
 
 #include <pthread.h>
 #include <stdio.h>
 #include <unistd.h>
 #include <stdlib.h>
 
 #define LOOPNUM 10
 #define THNUM 5
 
void enter(int *thnum)
{
  int loop;
 
  for (loop = 0; loop < LOOPNUM; loop++) {
    printf("%d (loop = %d): Enter Critical Region\n", *thnum, loop);
    usleep(rand() % 100000+100000);  // クリティカルリージョンの代わり
    printf("%d (loop = %d): Exit Critical Region\n", *thnum, loop);
    usleep(rand() % 500000+500000);
  }
  pthread_exit(NULL);
}
 
int main()
{
  int i;
  pthread_t th[THNUM];
  int params[THNUM];
 
  for(i = 0 ; i < THNUM ; i++) {
    params[i] = i;
    pthread_create(&th[i], NULL, (void (*))enter, &params[i]);
  }
  for(i = 0 ; i < THNUM ; i++) {
    pthread_join(th[i], NULL);
  }
  pthread_exit(NULL);
}
