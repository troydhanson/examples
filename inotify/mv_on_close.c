#include <sys/inotify.h>
#include <limits.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

/* usage: mv_on_close <watch-dir> <dest-dir>
 *
 * whenever a file in watch-dir is closed (if it was open for writing),
 * it is moved into the dest-dir.
 *
 * This implementation is not robust- it only works within a filesystem!
 */

int main(int argc, char *argv[]) {
  int fd, wd, mask, rc;
  char *dir, *dest, *name, oldname[PATH_MAX],newname[PATH_MAX];

  if (argc != 3) {
    fprintf(stderr,"usage: %s <watch-dir> <dest-dir>\n", argv[0]);
    exit(-1);
  }

  dir = argv[1];  int olen = strlen(dir);
  dest = argv[2]; int dlen = strlen(dest);
  memcpy(newname, dest, dlen); newname[dlen]='/';
  memcpy(oldname, dir, olen); oldname[olen]='/';

  if ( (fd = inotify_init()) == -1) {
    perror("inotify_init failed");
    exit(-1); 
  }

  mask = IN_CLOSE_WRITE;
  if ( (wd = inotify_add_watch(fd, dir, mask)) == -1) {
    perror("inotify_add_watch failed");
    exit(-1); 
  }

  /* see inotify(7) as inotify_event has a trailing name
   * field allocated beyond the fixed structure; we must
   * allocate enough room for the kernel to populate it */
  struct inotify_event *eb, *ev, *nx;
  size_t eb_sz = sizeof(*eb) + PATH_MAX, sz;
  if ( (eb = malloc(eb_sz)) == NULL) {
    fprintf(stderr, "out of memory\n");
    exit(-1);
  }

  /* one read will produce one or more event structures */
  while ( (rc=read(fd,eb,eb_sz)) > 0) {
    for(ev = eb; rc > 0; ev = nx) {

      sz = sizeof(*ev) + ev->len;
      nx = (struct inotify_event*)((char*)ev + sz);
      rc -= sz;

      name = (ev->len ? ev->name : dir);
      memcpy(&newname[dlen+1],name,strlen(name)+1);
      memcpy(&oldname[olen+1],name,strlen(name)+1);
      fprintf(stderr, "%s --> %s\n", oldname, newname); 

      if (rename(oldname, newname)) {
	fprintf(stderr,"failed to rename: %s\n",strerror(errno));
      }
#if 0
      if (ev->mask & IN_ACCESS) printf(" IN_ACCESS");
      if (ev->mask & IN_MODIFY) printf(" IN_MODIFY");
      if (ev->mask & IN_ATTRIB) printf(" IN_ATTRIB");
      if (ev->mask & IN_CLOSE_WRITE) printf(" IN_CLOSE_WRITE");
      if (ev->mask & IN_CLOSE_NOWRITE) printf(" IN_CLOSE_NOWRITE");
      if (ev->mask & IN_OPEN) printf(" IN_OPEN");
      if (ev->mask & IN_MOVED_FROM) printf(" IN_MOVED_FROM");
      if (ev->mask & IN_MOVED_TO) printf(" IN_MOVED_TO");
      if (ev->mask & IN_CREATE) printf(" IN_CREATE");
      if (ev->mask & IN_DELETE) printf(" IN_DELETE");
      if (ev->mask & IN_DELETE_SELF) printf(" IN_DELETE_SELF");
      if (ev->mask & IN_MOVE_SELF) printf(" IN_MOVE_SELF");
      printf("\n");
#endif
    }
  }

  close(fd);
}
