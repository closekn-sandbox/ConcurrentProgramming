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

//-- func
void err_msg(char *msg);

int main(int argc, char *argv[]) {
  char *sname;                 // argv[1] server name
  char identifier[IDENT_BYTE]; // argv[2] identifier
  struct hostent *retrieve;
  struct sockaddr_in serv_addr;
  int ipaddr;                  // server IP
  int port_no;                 // port number
  int sockfd;                  // socket
  // flag
  int join;

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
    
  // 能動オープン(コネクション要求)
  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    err_msg("client: can't connect server address");
  }
  fcntl(fileno(stdin), F_SETFL, O_NONBLOCK);
  fcntl(fileno(stdout), F_SETFL, O_NONBLOCK);
  fcntl(sockfd, F_SETFL, O_NONBLOCK);

  //-- janken
  while ( 1 ) {
    
    break;//
  }

  // end
  close(sockfd);

  return 0;
}

// エラーメッセージ
void err_msg(char *msg) {
  perror(msg);
  exit(1);
}