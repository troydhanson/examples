#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define R  0
#define W  1

int main(int argc, char *argv[]) {
  char buf[100], *msg;
  int fd[2], sc;
  ssize_t nr;
  pid_t pid;

  sc = pipe(fd);
  if (sc < 0) {
    fprintf(stderr,"pipe: %s\n", strerror(errno));
    exit(-1);
  }

  pid = fork();
  if (pid < 0) {
    fprintf(stderr,"fork: %s\n", strerror(errno));
    exit(-1);
  }

  if (pid > 0) { /* parent */

    /* parent close read end */
    close( fd[R] );

    msg = "hello, world!";
    nr = write(fd[W], msg, strlen(msg));
    if (nr < 0) {
      fprintf(stderr,"write: %s\n", strerror(errno));
      exit(-1);
    }

    close( fd[W] );
    wait(NULL);

  } else {       /* child */

    /* child close write end */
    close( fd[W] );

    do {

      fprintf(stderr, "child: reading\n");
      nr = read(fd[R], buf, sizeof(buf));
      if      (nr <  0) fprintf(stderr,"read: %s\n", strerror(errno));
      else if (nr == 0) fprintf(stderr,"read: eof\n");
      else    fprintf(stderr, "child: read %.*s\n", (int)nr, buf);

    } while( nr > 0);
  }

  return 0;
}
