#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

/*
 *  demonstrate opening a POSIX message queue and reading from it.
 *
 *  usage:
 *
 *     ./mq-open /foo
 *
 *  see:
 *     mq_overview(7)
 *     mq_open(3)
 *     mq_receive(3)
 *
 */

#define MSG_MAX 8192 /* minimum in some kernels, see mq_overview(7) */

struct {
  char *prog;
  int verbose;
  char *name;
  mqd_t fd;
} CF;

void usage(void) {
  fprintf(stderr,"usage: %s [-v] <name>\n", CF.prog);
  exit(-1);
}

int main(int argc, char *argv[]) {
  char buf[MSG_MAX];
  int opt, rc=-1;
  ssize_t nr;

  CF.prog = argv[0];

  while ( (opt = getopt(argc,argv,"vh")) > 0) {
    switch(opt) {
      case 'v': CF.verbose++; break;
      case 'h': default: usage(); break;
    }
  }

  if (optind >= argc) usage();

  struct mq_attr attr = {
    .mq_maxmsg = 10,        /* max messages on queue */
    .mq_msgsize = MSG_MAX,  /* max size of a message */
  };

  CF.name = argv[optind++];
  CF.fd = mq_open(CF.name, O_RDONLY | O_CREAT, 0600, &attr);
  if (CF.fd == (mqd_t)-1) {
    fprintf(stderr,"mq_open %s: %s\n", CF.name, strerror(errno));
    goto done;
  }

  nr = mq_receive(CF.fd, buf, MSG_MAX, NULL);
  if (nr < 0) {
    fprintf(stderr, "mq_receive: %s\n", strerror(errno));
    goto done;
  }

  printf("received %d bytes: %.*s\n", (int)nr, (int)nr, buf);

  rc = 0;
 
 done:
  if (CF.fd != (mqd_t)-1) mq_close(CF.fd);
  return rc;
}
