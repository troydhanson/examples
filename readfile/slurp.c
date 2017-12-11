#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
 
/* read a file, placing its size into len, returning buffer or NULL on error.
 * caller should free the buffer eventually.
 */
char *slurp(char *file, size_t *len) {
  int fd = -1, rc = -1, sc;
  char *buf=NULL;
  struct stat s;
  ssize_t nr;

  sc = stat(file, &s);
  if (sc < 0) {
    fprintf(stderr,"stat: %s\n", strerror(errno));
    goto done;
  }

  *len = s.st_size;
  fd = open(file, O_RDONLY);
  if (fd < 0) {
    fprintf(stderr,"open: %s\n", strerror(errno));
    goto done;
  }

  buf = malloc(*len);
  if (buf == NULL) {
    fprintf(stderr, "out of memory\n");
    goto done;
  }

  nr = read(fd, buf, *len);
  if (nr != (ssize_t)*len) {
    fprintf(stderr,"read: incomplete\n");
    goto done;
  }

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
  buf = slurp(file, &len);

  if (buf) {
    printf("slurped %s: %u bytes\n", file, (unsigned)len);
    free(buf);
  }

  return 0;
}
