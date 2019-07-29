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
#define MAX_CHNUM     2

//-- func
void err_msg(char *msg);

//-- main
int main(int argc, char *argv[]) {
  struct sockaddr_in serv_addr, cli_addr;
  socklen_t cli_len = sizeof(cli_addr);
  int port_no;            // ポート番号
  int sockid;             // コネクション待ちソケット
  int sockfd[MAX_CHNUM];  // 通信用ソケット
  // user
  int chnum = 0;          // child num

  // rep
  int i;
  

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
  while ( 1 ) {
    while ( chnum < MAX_CHNUM ) {
      fprintf(stderr ,"あと%d名\n", MAX_CHNUM-chnum);
      // クライアントからコネクションの要求を受け入れる 
      if ((sockfd[chnum] = accept(sockid, (struct sockaddr *)&cli_addr, &cli_len)) < 0) {
        close(sockid);
        fprintf(stderr, "Server: can't accept");
        break;
      }
      chnum++;
    }
    
    break;  //
  }

  // end
  close(sockid);
  for ( int i = 0; i < MAX_CHNUM; i++ ) { close(sockfd[i]); }

  return 0;
}

// エラーメッセージ
void err_msg(char *msg) {
  perror(msg);
  exit(1);
}