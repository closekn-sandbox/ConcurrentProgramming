#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>
#include "MT.h"

#define	SERV_TCP_PORT 20000 // port_no
#define SLEEP_TIME    4000  // ms
#define BYTE          100   // msg byte
#define IDENT_BYTE    10    // ident byte

//-- func
void err_msg(char *msg);
int choose_hand();

int main(int argc, char *argv[]) {
  char *sname;                 // argv[1] server name
  char identifier[IDENT_BYTE]; // argv[2] identifier
  struct hostent *retrieve;
  struct sockaddr_in serv_addr;
  int ipaddr;                  // server IP
  int port_no;                 // port number
  int sockfd;                  // socket
  char read_msg[BYTE];
  char send_msg[BYTE];
  // flag
  int next_play;
  int next_hand;
  int next_result;
  int next_read;
  int next_send;

  //init rand
  init_genrand((unsigned)time(NULL));

  // 引数不足 > ./child <server-name> <identifier>
  if ( argc < 3 ) {
    fprintf(stderr, "Usage : %s <server_name> <identifier>\n", argv[0]);
    exit(1);
  }

  // サーバ
  sname = argv[1];
  // サーバ見つからない場合エラー1
  if ((retrieve = gethostbyname(sname)) == NULL) {
    fprintf(stderr, "Unknown host name: %s\n", sname);
    exit(1);
  }
  // サーバIP取得 ポート取得
  ipaddr = *(unsigned int *)(retrieve->h_addr_list[0]);
  port_no = SERV_TCP_PORT;
  // ローカルIP取得
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = *(unsigned int *)(retrieve->h_addr_list[0]);
  serv_addr.sin_port = htons(port_no);

  // 識別子
  sprintf(identifier, "%s", argv[2]);
  if ( strlen(identifier) > IDENT_BYTE ) {
    fprintf(stderr, "Over identifier size: %s\n", identifier);
    exit(1);
  }

  // ソケットの作成(TCP接続)
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    err_msg("client: can't open datastream socket");
  }
    
  // 能動オープン(コネクション要求) join
  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    err_msg("client: can't connect server address");
  }
  fcntl(fileno(stdin), F_SETFL, O_NONBLOCK);
  fcntl(fileno(stdout), F_SETFL, O_NONBLOCK);
  fcntl(sockfd, F_SETFL, O_NONBLOCK);

  //-- janken
  next_play = 1;
  next_hand = 1;
  next_result = 0;
  next_read = 1;
  next_send = 0;
  while ( next_play ) {
    // hand
    while ( next_hand ) {
      if ( next_read ) {
        if ( recv(sockfd, read_msg, BYTE, 0) != -1 ) {
          next_read = 0; next_send = 1;
        }
      }
      if ( next_send ) {
        int res; // ts
        fprintf(stderr, "%s\n", read_msg);
        memset(&send_msg, 0x0, BYTE); /* pading */
        sprintf(send_msg, "%s %d ", identifier, choose_hand());
        send(sockfd, send_msg, BYTE, 0);
        next_send = 0; next_read = 1;
        next_hand = 0; next_result = 1;
      }
    }

    // result
    while ( next_result ) {
      if ( recv(sockfd, read_msg, BYTE, 0) != -1 ) {
        fprintf(stderr, "%s\n", read_msg);
        next_read = 0; next_send = 1;
        next_result = 0;
      }
    }

    if ( recv(sockfd, read_msg, BYTE, 0) != -1 ) {
      sscanf(read_msg, "%d", &next_play);
      if ( next_play ) { next_hand = 1; }
    }
  }

  // end
  close(sockfd);

  return 0;
}

// 0-2の乱数生成
int choose_hand() {
  return genrand_int32()%3;
}

// エラーメッセージ
void err_msg(char *msg) {
  perror(msg);
  exit(1);
}