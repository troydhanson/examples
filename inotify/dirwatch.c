#include <sys/inotify.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

/* start this program with a directory argument.
 * then go into that directory and create, remove,
 * rename, or read and write to files and watch
 * as inotify receives the event notifications.
 * Requires Linux 2.6.13 or higher */

int main(int argc, char *argv[]) {
  int fd, wd, mask, rc;
  char *dir, *name;

  if (argc != 2) {
    fprintf(stderr,"usage: %s <dir>\n", argv[0]);
    exit(-1);
  }

  dir = argv[1];

  if ( (fd = inotify_init()) == -1) {
    perror("inotify_init failed");
    exit(-1); 
  }

  mask = IN_ALL_EVENTS;
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
      printf("%s ", name);
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
      if (ev->mask & IN_IGNORED) printf(" IN_IGNORED");
      if (ev->mask & IN_ISDIR) printf(" IN_ISDIR");
      if (ev->mask & IN_Q_OVERFLOW) printf(" IN_Q_OVERFLOW");
      if (ev->mask & IN_UNMOUNT) printf(" IN_UNMOUNT");
      printf("\n");
    }
  }

  close(fd);
}
