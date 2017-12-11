#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

/*
 *
 * simple example of reading a directory and doing a suffix match on files
 *
 * Example usage:
 *
 *  ./readdir-suffix -d /etc -s .conf
 *
 */

struct {
  int verbose;
  char *dir;
  char *suffix;
  char *prog;
} CF = {
};

void usage() {
  fprintf(stderr, "usage: %s [-v] [-s <suffix>] -d <dir>\n", CF.prog);
  exit(-1);
}

int match_suffix(char *file, char *suffix) {
  size_t file_len, suffix_len;
  char *file_suffix;

  /* not enforcing suffix match? */
  if (suffix == NULL) return 1;

  file_len = strlen(file);
  suffix_len = strlen(suffix);

  /* file too short for suffix match? */
  if (file_len < suffix_len) return 0;

  file_suffix = &file[ file_len - suffix_len ];
  return strcmp(file_suffix, suffix) ? 0 : 1;
}

int main(int argc, char *argv[]) {
  struct dirent *dent;
  int opt, rc=-1;
  DIR *d = NULL;
  char *file;

  CF.prog = argv[0];

  while ( (opt = getopt(argc,argv,"vhd:s:")) > 0) {
    switch(opt) {
      case 'v': CF.verbose++; break;
      case 'd': CF.dir = strdup(optarg); break;
      case 's': CF.suffix = strdup(optarg); break;
      case 'h': default: usage(argv[0]); break;
    }
  }

  if (CF.dir == NULL) usage();

  d = opendir(CF.dir);
  if (d == NULL) {
    fprintf(stderr, "opendir %s: %s\n", CF.dir, strerror(errno));
    goto done;
  }

  while ( (dent = readdir(d)) != NULL) {
    file = dent->d_name;
    if (match_suffix(file, CF.suffix) == 0) continue;
    fprintf(stderr, "suffix match: %s\n", file);
  }

  rc = 0;
 
 done:
  if (d) closedir(d);
  return rc;
}
