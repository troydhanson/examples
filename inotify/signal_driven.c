#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/inotify.h>
#include <signal.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <string.h>

/* signals that we'll accept during sigwaitinfo */
int sigs[] = {SIGIO,SIGHUP,SIGTERM,SIGINT,SIGALRM};

void usage(char *prog) {
  fprintf(stderr,"usage: %s dir [dir ...]\n", prog);
  exit(-1);
}

const size_t eb_sz = sizeof(struct inotify_event) + PATH_MAX;

int setup_watch(int argc, char *argv[], int fd, struct inotify_event **eb) {
  int i,wd,mask=IN_ALL_EVENTS;
  char *dir;

  for(i=1; i < argc; i++) {
    dir = argv[i];
    if ( (wd = inotify_add_watch(fd, dir, mask)) == -1) {
      perror("inotify_add_watch failed");
      exit(-1);
    }
  }
  if ( (*eb = malloc(eb_sz)) == NULL) {
    fprintf(stderr,"out of memory\n");
    exit(-1);
  }
}

int read_events(int fd, struct inotify_event *eb) {
  struct inotify_event *ev, *nx;
  char *dir = "<dir>", *name;
  size_t sz;
  int rc;

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
      printf("\n");
    }
  }
  return rc;
}

int main(int argc, char *argv[]) {
  int fd, rc, n, signo, num_bytes;
  struct inotify_event *eb;

  if (argc < 2) usage(argv[0]);

  if ( (fd = inotify_init()) == -1) {
    perror("inotify_init failed");
    exit(-1);
  }

  /* request SIGIO to our pid when fd is ready; see fcntl(2) */
  int fl = fcntl(fd, F_GETFL);
  fl |= O_ASYNC | O_NONBLOCK;
  fcntl(fd, F_SETFL, fl);
  fcntl(fd, F_SETOWN, getpid());
  fcntl(fd, F_SETSIG, SIGIO);

  /* block all signals. stay blocked except in sigwaitinfo */
  sigset_t all;
  sigfillset(&all);
  sigprocmask(SIG_SETMASK,&all,NULL);

  /* a few signals we'll accept during sigwaitinfo */
  sigset_t sw;
  sigemptyset(&sw);
  for(n=0; n < sizeof(sigs)/sizeof(*sigs); n++) sigaddset(&sw, sigs[n]);

  setup_watch(argc,argv,fd,&eb);

  siginfo_t info;
  alarm(1);

  while ( (signo = sigwaitinfo(&sw, &info)) > 0) {
    switch(signo) {
      case SIGALRM:
        alarm(1);
        /* many kernels even into 3.x do not reliably generate
         * the signal on inotify descriptors. so we check whether
         * there are bytes available on that descriptor manually */
        if (ioctl(fd,FIONREAD,&num_bytes) != 0) {
          fprintf(stderr,"ioctl error %s\n",strerror(errno));
          goto done;
        }
        if (num_bytes == 0) break;
        fprintf(stderr,"unsignaled data on inotify descriptor (%u bytes)\n",num_bytes);
        /* so, fall through */
      case SIGIO:
        rc = read_events(fd,eb);
        if (rc == 0) goto done;
        if ((rc == -1) && (errno == EAGAIN)) break; /* no data */
        if (rc == -1) {
          fprintf(stderr,"read error %s\n", strerror(errno));
          goto done;
        }
        break;
      default:
        fprintf(stderr,"got signal %d\n", signo);
        goto done;
        break;
    }
  }

  done:
    close(fd);
    return 0;
}
