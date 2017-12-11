#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

/*
 *  demonstrate unlinking a POSIX message queue 
 *
 *  usage:
 *
 *     ./mq-unlink /foo
 *
 *  see:
 *     mq_overview(7)
 *     mq_unlink(3)
 *
 */

struct {
  char *prog;
  int verbose;
  char *name;
} CF;

void usage(void) {
  fprintf(stderr,"usage: %s [-v] <name>\n", CF.prog);
  exit(-1);
}

int main(int argc, char *argv[]) {
  int opt, rc=-1, sc;

  CF.prog = argv[0];

  while ( (opt = getopt(argc,argv,"vh")) > 0) {
    switch(opt) {
      case 'v': CF.verbose++; break;
      case 'h': default: usage(); break;
    }
  }

  if (optind >= argc) usage();

  CF.name = argv[optind++];
  sc = mq_unlink(CF.name);
  if (sc == (mqd_t)-1) {
    fprintf(stderr,"mq_unlink %s: %s\n", CF.name, strerror(errno));
    goto done;
  }

  rc = 0;
 
 done:
  return rc;
}
