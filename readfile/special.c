#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/fcntl.h>
 
/* read a file of unknown size, such as special files in /proc. 
 * place its size into len, returning buffer or NULL on error.
 * caller should free the buffer eventually.
 */
char *slurp_special(char *file, size_t *len) {
  char *buf=NULL, *b, *tmp;
  int fd = -1, rc = -1, eof=0;
  size_t sz, br=0, l;
  ssize_t nr;

  /* initial guess at a sufficient buffer size */
  sz = 1000;

  fd = open(file, O_RDONLY);
  if (fd < 0) {
    fprintf(stderr,"open: %s\n", strerror(errno));
    goto done;
  }

  while(!eof) {

    tmp = realloc(buf, sz);
    if (tmp == NULL) {
      fprintf(stderr, "out of memory\n");
      goto done;
    }

    buf = tmp;
    b = buf + br;
    l = sz - br;

    do {
      nr = read(fd, b, l);
      if (nr < 0) {
        fprintf(stderr,"read: %s\n", strerror(errno));
        goto done;
      }

      b += nr;
      l -= nr;
      br += nr;

      /* out of space? double buffer size */
      if (l == 0) { 
        sz *= 2;
        fprintf(stderr, "realloc to %lu\n", sz);
        break;
      }

      if (nr == 0) eof = 1;

    } while (nr > 0);
  }

  *len = br;
  rc = 0;

 done:
  if (fd != -1) close(fd);
  if (rc && buf) { free(buf); buf = NULL; }
  return buf;
}

int main(int argc, char * argv[]) {
  char *file, *buf;
  size_t len;;
 
  if (argc < 2) {
    fprintf(stderr, "usage: %s <file>\n", argv[0]);
    exit(-1);
  }

  file = argv[1];
  buf = slurp_special(file, &len);

  if (buf) {
    printf("slurped %s: %u bytes\n", file, (unsigned)len);
    free(buf);
  }

  return 0;
}
