#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

#define	SERV_TCP_PORT 20000 // port_no
#define BYTE          100   // msg byte
#define NAME_LEN      10
#define MAX_JOIN      4
#define SEND_HAND     "\nグー:0 チョキ:1 パー:2\n> "
#define ONE_MORE      "\nもう一回? YES:1 NO:0\n> "

char* hand_kind[3] = { "グー", "チョキ", "パー" };
char* result_kind[3] = { "勝ち", "負け", "引き分け" };

//-- func
void err_msg(char *msg);
void janken_result(int hand[], int result[], int n);

//-- main
int main(int argc, char *argv[]) {
  struct sockaddr_in serv_addr, cli_addr;
  socklen_t cli_len = sizeof(cli_addr);
  int port_no;            // ポート番号
  int sockid;             // コネクション待ちソケット
  int sockfd[MAX_JOIN];  // 通信用ソケット
  int rdnum;              // read num
  char read_msg[BYTE];    // read message
  char send_msg[BYTE];    // send message
  // user
  int join_num = 1;       // the number of joining members
  char name[MAX_JOIN][NAME_LEN];    // child name
  int hand[MAX_JOIN];    // グー:0 チョキ:1 パー:2
  int result[MAX_JOIN];  // win:0 lose:1 draw:2
  // rep
  int i;
  // flag
  int next_play;
  int next_join;
  int next_hand;
  int next_result;
  int next_read;
  int next_send;
  

  // 引数不足

  // ログファイル指定

  // コネクション要求受付用のソケット確保
  if ((sockid = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    err_msg("server: can't open datastream socket");
  }
  // ソケットとポートの対応づけ
  port_no = SERV_TCP_PORT;
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family	  = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port	  = htons(port_no);
  if (bind(sockid, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    err_msg("server: can't bind local address");
  }
  // 受動オープン状態へ移行
  if (listen(sockid, 5) == -1) {
    err_msg("server: listen failed");
  }

  //-- janken
  // init flag
  next_play = 1;
  next_join = 1;
  next_hand = 0;
  next_result = 0;
  next_read = 0;
  next_send = 0;

  // wait join
  while ( next_join ) {
    fprintf(stderr ,"あと%d名\n", MAX_JOIN-join_num);
    // クライアントからコネクションの要求を受け入れる 
    if ((sockfd[join_num] = accept(sockid, (struct sockaddr *)&cli_addr, &cli_len)) < 0) {
      close(sockid);
      fprintf(stderr, "Server: can't accept");
      break;
    }
    join_num++;
    if ( join_num >= MAX_JOIN ) { next_join = 0; next_hand = 1; next_send = 1; }
  }

  //-- play
  while ( next_play ) {
    // hand
    while ( next_hand ) {
      if ( next_send ) {
        fprintf(stderr, "%s", SEND_HAND);
        fscanf(stdin, "%d", &hand[0]);
        for ( int i = 1; i < join_num; i++ ) {
          memset(&send_msg, 0x0, BYTE);
          sprintf(send_msg, "play");
          send(sockfd[i], send_msg, BYTE, 0);
        }
        next_send = 0; next_read = 1;
      }
      if ( next_read ) {
        int all_read = 1;
        for ( int i = 1; i < join_num; i++ ) {
          // 手の読み取り
          if ( recv(sockfd[i], read_msg, BYTE, 0) != -1 ) {
            sscanf(read_msg, "%s %d", name[i], &hand[i]);
            fprintf(stderr, "%s : %s\n", name[i], hand_kind[hand[i]]);
          } else { all_read = 0; }
        }
        if ( all_read ) { next_read = 0; next_send = 1; }
      }
      if ( next_send ) { next_hand = 0; next_result = 1; }
    }
    
    janken_result(hand, result, join_num);
    fprintf(stderr, "あなたは %s\n", result_kind[result[0]]);

    // send result
    while ( next_result ) {
      if ( next_send ) {
        for ( int i = 1; i < join_num; i++ ) {
          memset(&send_msg, 0x0, BYTE);
          sprintf(send_msg, "%s %s", name[i], result_kind[result[i]]);
          send(sockfd[i], send_msg, BYTE, 0);
        }
        next_send = 0; next_read = 1;
      }
      if ( next_read == 1 ) {
        next_result = 0; next_hand = 1;
        next_read = 0; next_send = 1;  
      }
    }
    
    fprintf(stderr, ONE_MORE);
    fscanf(stdin, "%d", &next_play);
    for ( i = 1; i < join_num; i++ ) {
      memset(&send_msg, 0x0, BYTE);
      sprintf(send_msg, "%d ", next_play);
      send(sockfd[i], send_msg, BYTE, 0);
    }
  }

  // end
  close(sockid);
  for ( int i = 1; i < MAX_JOIN; i++ ) { close(sockfd[i]); }

  return 0;
}

// judgment
void janken_result(int hand[], int result[], int n) {
  int judg = 0;
  int win_hand;
  int i;
  for ( i = 0; i < n; i++ ) {
    judg |= 1<<hand[i];
  }
  // draw
  if ( judg == 1 || judg == 2 || judg == 4 || judg == 7 ) {
    for ( i = 0; i < n; i++ ) { result[i] = 2; }
    return;
  }
  // win lose
  if ( judg == 3 ) { win_hand = 0; }
  if ( judg == 5 ) { win_hand = 2; }
  if ( judg == 6 ) { win_hand = 1; }
  for ( i = 0; i < n; i++ ) {
    if ( hand[i] == win_hand ) { result[i] = 0; } else { result[i] = 1; }
  }
}

// エラーメッセージ
void err_msg(char *msg) {
  perror(msg);
  exit(1);
}