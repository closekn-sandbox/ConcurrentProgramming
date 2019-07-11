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

#define	SERV_TCP_PORT 20000
#define BYTE          100

void err_msg(char *msg); // エラーメッセージ

int main(int argc, char *argv[]) {
  int fd;               // オープンされるファイルディスクリプタ
  char *fname;          // log file
  struct sockaddr_in serv_addr, cli_addr;
  socklen_t cli_len = sizeof(cli_addr);
  int port_no;          // ポート番号
  int sockid;           // コネクション待ちソケット
  int sockfd;           // 通信用ソケット
  char read_msg[BYTE];  // read msg
  char send_msg[BYTE];  // send msg
  int rdnum;            // read num
  int pid;              // fork
  int chnum = 0;        // number of child processes
  int status;

  // 引数によるファイル名指定なしでエラー2終了
  if ( argc < 2 ) {
    fprintf(stderr, "Usage : %s file_name\n", argv[0]);
    exit(2);
  }
  // サイズ0の指定ファイル作成
  fname = argv[1];
  fd = open(fname, O_RDWR | O_CREAT, 0644);
  close(fd);
  // コネクション要求受付用のソケット確保
  if ((sockid = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    err_msg("srever: can't open datastream socket");
  }
  // ソケットとポートの対応づけ
  port_no = SERV_TCP_PORT;
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family	  = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port	  = htons(port_no);
  if (bind(sockid, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    err_msg("srever: can't bind local address");
  }
  // 受動オープン状態へ移行
  if (listen(sockid, 5) == -1) {
    err_msg("srever: listen failed");
  }
  
  //-- 反復
  while ( 1 ) {
    if (chnum) {
      /* for terminating child process */
      while (chnum && ((pid = waitpid(-1, &status, WNOHANG)) > 0)) {
	      fprintf(stderr, "Terminate child process: %d\n", pid);
	      chnum--;
      }
    }
    // クライアントからコネクションの要求を受け入れる 
    if ((sockfd = accept(sockid, (struct sockaddr *)&cli_addr, &cli_len)) < 0) {
      close(sockid);
      fprintf(stderr, "Server: can't accept");
      break;
    }
    //-- forkする
    /* fork error */
    pid = fork();
    if (pid < 0) {
      close(sockfd);
      close(sockid);
      break;
    }
    /* parent process */
    if (pid > 0) {
      // 通信用のソケット (accept 関数の戻り値として得られたソケット) を閉じる
      close(sockfd);
      chnum++;
      continue;
    } 
    /* child process */
    pid = getpid();
    fprintf(stderr, "\nI an child process %d\n", pid);
    // コネクション待ちのソケット (socket 関数の戻り値として得られたソケット) を閉じる
    close(sockid);

    while ( 1 ) {
      // 指定ファイルをオープンする
      if ((fd = open(fname, O_RDWR | O_CREAT, 0644)) == -1) {
        err_msg(fname);
      }
      // 指定ファイルを排他ロックする
#ifdef WITH_FLOCK
      if (flock(fd, LOCK_EX) == -1) {
        close(fd);
        err_msg(fname);
      }
#endif
      // ファイル読込
      lseek(fd, -(BYTE+1), SEEK_END);
      rdnum = read(fd, read_msg, BYTE);
      // 出力の作成
      memset(&send_msg, 0x0, BYTE);
      if ( rdnum == 0 ) {
        /* file empty */
        sprintf(send_msg, "0 NONE NONE");
      } else {
        strcpy(send_msg, read_msg);
      }
      fprintf(stderr, "%s\n", send_msg);
      // メッセージ送受信
      send(sockfd, send_msg, BYTE, 0);
      rdnum = recv(sockfd, read_msg, BYTE, 0);
      // ファイル書込
      if ( rdnum == BYTE ) {
        lseek(fd, 0, SEEK_END);
        write(fd, read_msg, BYTE);
        write(fd, "\n", 1);
      } else { break; }
      // 指定ファイルのロックを解除する
#ifdef WITH_FLOCK
	    flock(fd, LOCK_UN);
#endif
      // 指定ファイル,ソケットを閉じる
      close(fd);
      close(sockfd);
    }
  }

  return 0;
}

// エラーメッセージ
void err_msg(char *msg) {
  perror(msg);
  exit(1);
}