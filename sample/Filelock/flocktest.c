#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>

// When you want to use file lock, complie with -DWITH_FLOCK option

#define BLOCKSIZE 100

#define WRNUM 100
#define CRIT   50000 //  50ms (Average thinkng time in critical section)
#define OTHER 200000 // 200ms (Average thinking time in other part)

char buf[BLOCKSIZE];

int main(int argc, char **argv)
{
  int fd;
  int val;
  int oldpid;
  int rdnum;
  int loop;

  if (argc < 2) {
    fprintf(stderr, "Usage : %s file_name\n", argv[0]);
    exit(2);
  }
  srand(time(NULL));
  for (loop = 0; loop < WRNUM; loop++) {
    if ((fd = open(argv[1], O_RDWR | O_CREAT, 0644)) == -1) {
      perror(argv[1]);
      exit(1);
    }
#ifdef WITH_FLOCK
    if (flock(fd, LOCK_EX) == -1) {
      perror(argv[1]);
      exit(1);
    }
#endif
    if (lseek(fd, -BLOCKSIZE, SEEK_END) < 0) {
      val = 0;
      oldpid = -1;
    } else {
      if ((rdnum = read(fd, buf, BLOCKSIZE)) == -1) {
	perror(argv[1]);
#ifdef WITH_FLOCK
	flock(fd, LOCK_UN);
#endif
	exit(1);
      } else {
	buf[BLOCKSIZE-1] = '\0'; // stopper
	if (sscanf(buf, "%d %d", &oldpid, &val) != 2) {
	  fprintf(stderr, "%s: format error\n", buf);
#ifdef WITH_FLOCK
	  flock(fd, LOCK_UN);
#endif
	  exit(1);
	}
      }
    }
    usleep((rand() % CRIT) + CRIT/2);
    sprintf(buf, "%d %d (update value written by %d\n", getpid(),val+1, oldpid);
    write(fd, buf, BLOCKSIZE);
    fputs(buf, stdout);
#ifdef WITH_FLOCK
    flock(fd, LOCK_UN);
#endif
    close(fd);
    usleep((rand() % OTHER) + OTHER/2);
  }
  return 0;
}    
