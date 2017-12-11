#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/eventfd.h>
#include <inttypes.h>

int main(int argc, char *argv[]) {
  unsigned int initval;
  int fd, flags, posts=0;
  uint64_t u,v;
  ssize_t nr;
  pid_t pid;

  initval = 10;
  flags = EFD_SEMAPHORE|EFD_NONBLOCK;
  fd = eventfd(initval, flags);
  if (fd < 0) {
    fprintf(stderr,"eventfd: %s\n", strerror(errno));
    goto done;
  }

  pid = fork();
  if (pid < 0) {
    fprintf(stderr,"fork: %s\n", strerror(errno));
    goto done;
  }

  if (pid > 0) { /* parent */

    fprintf(stderr, "parent: writing\n");

    u = 3;
    nr = write(fd, &u, sizeof(u));
    if (nr < 0) {
      fprintf(stderr,"write: %s\n", strerror(errno));
      goto done;
    }

    wait(NULL);

  } else {       /* child */

    sleep(1);
    fprintf(stderr, "child: reading\n");
    while (1) {
      nr = read(fd, &v, sizeof(v));
      if (nr < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) break;
        fprintf(stderr,"read: %s\n", strerror(errno));
        goto done;
      } else if (nr > 0) {
        fprintf(stderr, "child: read %lu\n", (unsigned long)v);
        posts++;
      }
    }

    fprintf(stderr, "%u posts\n", posts);
  }

 done:
  return 0;
}
