#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

#define	SERV_TCP_PORT 20000 // port_no
#define SLEEP_TIME    4000  // ms
#define BYTE          100   // msg byte
#define IDENT_BYTE    35    // ident byte

void err_msg(char *msg); // エラーメッセージ

int main(int argc, char *argv[]) {
  char *sname;                 // argv[1] server name
  char identifier[IDENT_BYTE]; // argv[2] identifier
  int rcount;                  // argv[3] repeat count
  int ipaddr;                  // server IP
  int port_no;                 // port number
  int sockfd;                  // socket
  struct hostent *retrieve;
  struct sockaddr_in serv_addr;
  char ch;
  int rflag, wflag, endflag;
  char read_msg[BYTE];
  char send_msg[BYTE];

  // 引数不足でエラー2終了
  if ( argc < 4 ) {
    fprintf(stderr, "Usage : %s server_name identifier repeat_count\n", argv[0]);
    exit(2);
  }
  // サーバ名
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
  // 繰り返し回数not数値 エラー1
  rcount = atoi(argv[3]);
  if ( rcount == 0 ) {
    fprintf(stderr, "repeat_count is a number\n");
    exit(1);
  } 

  //-- rcount回反復
  while ( rcount-- > 0 ) {
    // ソケットの作成(TCP接続)
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      err_msg("client: can't open datastream socket");
    }
    
    // 能動オープン(コネクション要求)
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
      err_msg("client: can't connect server address");
    }
    fcntl(fileno(stdin), F_SETFL, O_NONBLOCK);
    fcntl(fileno(stdout), F_SETFL, O_NONBLOCK);
    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    // フラグ初期化
    rflag = 1; wflag = 0; endflag = 0;
    //-- 送受信処理
    while ( 1 ) {
      if ( rflag == 1 ) {
        if ( read(fileno(stdin), &ch, 1) == 1 ) { rflag = 0; }
      }
      if ( rflag == 0 ) {
        if ( write(sockfd, &ch, 1) == 1 ) { rflag = 1; }
      }
      if ( wflag == 0 ) {
        // メッセージ受信 と 受信メッセージから送信メッセージの作成
        if ( recv(sockfd, read_msg, BYTE, 0) != -1 ) {
          int number;
          char ident1[35];
          sscanf(read_msg, "%d %s", &number, ident1);
          memset(&send_msg, 0x0, BYTE); /* pading */
          sprintf(send_msg, "%d %s %s", number+1, identifier, ident1);
          wflag = 1;
        }
      }
      if ( wflag == 1 ) {
        // メッセージ送信
        fprintf(stderr, "%s\n", send_msg);
        if ( send(sockfd, send_msg, BYTE, 0) != -1 ) {
          endflag = 1;
        }
      }
      if ( endflag == 1 ) {
        endflag = 0;
        break;
      }
    }

    // 適当な時間待機後 ソケット閉(TCPコネクション切断)
    usleep(SLEEP_TIME);
    close(sockfd);
  }

  return 0;
}

// エラーメッセージ
void err_msg(char *msg) {
  perror(msg);
  exit(1);
}