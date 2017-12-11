#include <semaphore.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

struct {
  int verbose;
  char *prog;
  char *file;
  sem_t *sem;
} CF;

void usage() {
  fprintf(stderr,"usage: %s [-v] <file>\n", CF.prog);
  exit(-1);
}

int main(int argc, char *argv[]) {
  int opt, rc=-1;
  CF.prog = argv[0];
  int flags=0;

  while ( (opt = getopt(argc,argv,"vh")) > 0) {
    switch(opt) {
      case 'v': CF.verbose++; break;
      case 'h': default: usage(); break;
    }
  }

  if (argc > optind) CF.file = argv[optind++];
  else usage();

  CF.sem = sem_open(CF.file, flags);
  if (CF.sem == SEM_FAILED) {
    fprintf(stderr,"sem_open %s: %s\n", CF.file, strerror(errno));
    goto done;
  }
  
  if (sem_post(CF.sem) == -1) {
    fprintf(stderr,"sem_post %s: %s\n", CF.file, strerror(errno));
    goto done;
  }

  rc = 0;
 
 done:
  if (CF.sem && (CF.sem != SEM_FAILED)) sem_close(CF.sem);
  return rc;
}
