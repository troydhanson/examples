#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* example of readlink(2) */

int main(int argc, char *argv[]) {
  char *link, target[100];
  ssize_t sc;

  if (argc < 2) {
    fprintf(stderr,"usage: %s <symlink>\n", argv[0]);
    exit(-1);
  }

  link = argv[1];
  sc = readlink(link, target, sizeof(target));
  if (sc < 0) {
    fprintf(stderr,"readlink: %s\n", strerror(errno));
    exit(-1);
  }

  if (sc == sizeof(target)) fprintf(stderr, "truncation occurred!\n");

  /* print the first "sc" bytes of target. its NOT null-terminated!*/
  printf("%s -> %.*s\n", link, (int)sc, target);

  return 0;
}
