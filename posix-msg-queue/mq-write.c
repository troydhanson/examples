#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

/*
 *  demonstrate opening a POSIX message queue and writing to it.
 *
 *  usage:
 *
 *     ./mq-write /foo Hello
 *
 *  see:
 *     mq_overview(7)
 *     mq_open(3)
 *     mq_send(3)
 *
 */
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
  int opt, rc=-1, sc;
  char *msg;

  CF.prog = argv[0];

  while ( (opt = getopt(argc,argv,"vh")) > 0) {
    switch(opt) {
      case 'v': CF.verbose++; break;
      case 'h': default: usage(); break;
    }
  }

  if (optind >= argc) usage();

  CF.name = argv[optind++];
  CF.fd = mq_open(CF.name, O_WRONLY);
  if (CF.fd == (mqd_t)-1) {
    fprintf(stderr,"mq_open %s: %s\n", CF.name, strerror(errno));
    goto done;
  }

  msg = (optind < argc) ? argv[optind++] : "hello";
  sc = mq_send(CF.fd, msg, strlen(msg), 0);
  if (sc < 0) {
    fprintf(stderr, "mq_send: %s\n", strerror(errno));
    goto done;
  }

  rc = 0;
 
 done:
  if (CF.fd != (mqd_t)-1) mq_close(CF.fd);
  return rc;
}
