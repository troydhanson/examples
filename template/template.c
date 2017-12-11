#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

struct {
  char *prog;
  int verbose;
  char *file; /* file to read. if null, read stdin */
  int fd;     /* input file descriptor def.0=stdin */
} CF = {
  .fd = -1,
};

void usage() {
  fprintf(stderr,"usage: %s [-v] <file>\n", CF.prog);
  exit(-1);
}

int main(int argc, char *argv[]) {
  int opt, rc=-1;
  
  CF.prog = argv[0];

  while ( (opt = getopt(argc,argv,"vh")) > 0) {
    switch(opt) {
      case 'v': CF.verbose++; break;
      case 'h': default: usage(); break;
    }
  }

  if (optind >= argc) usage();

  CF.file = argv[optind++];
  CF.fd = open(CF.file ,O_RDONLY);
  if (CF.fd == -1) {
    fprintf(stderr,"open %s: %s\n", CF.file, strerror(errno));
    goto done;
  }

  rc = 0;
 
 done:
  if (CF.fd != -1) close(CF.fd);
  return rc;
}
