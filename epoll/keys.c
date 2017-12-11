/*
 * another epoll based sample program
 * this one reads the keyboard and signalfd
 */

#include <errno.h>
#include <termios.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

struct {
  int verbose;
  char *prog;
  int signal_fd;
  int epoll_fd;
  int ticks;
} cfg = {
  .signal_fd = -1,
  .epoll_fd = -1,
};

void usage() {
  fprintf(stderr,"usage: %s [-v] \n", cfg.prog);
  exit(-1);
}

/* signals that we'll accept via signalfd in epoll */
int sigs[] = {SIGHUP,SIGTERM,SIGINT,SIGQUIT,SIGALRM};

void periodic_work() {
  fprintf(stderr,"periodic work...\n");
}

int new_epoll(int events, int fd) {
  int rc;
  struct epoll_event ev;
  memset(&ev,0,sizeof(ev)); // placate valgrind
  ev.events = events;
  ev.data.fd= fd;
  if (cfg.verbose) fprintf(stderr,"adding fd %d to epoll\n", fd);
  rc = epoll_ctl(cfg.epoll_fd, EPOLL_CTL_ADD, fd, &ev);
  if (rc == -1) {
    fprintf(stderr,"epoll_ctl: %s\n", strerror(errno));
  }
  return rc;
}

int mod_epoll(int events, int fd) {
  int rc;
  struct epoll_event ev;
  memset(&ev,0,sizeof(ev)); // placate valgrind
  ev.events = events;
  ev.data.fd= fd;
  if (cfg.verbose) fprintf(stderr,"modding fd %d epoll\n", fd);
  rc = epoll_ctl(cfg.epoll_fd, EPOLL_CTL_MOD, fd, &ev);
  if (rc == -1) {
    fprintf(stderr,"epoll_ctl: %s\n", strerror(errno));
  }
  return rc;
}

int handle_signal() {
  int rc=-1;
  struct signalfd_siginfo info;
  
  if (read(cfg.signal_fd, &info, sizeof(info)) != sizeof(info)) {
    fprintf(stderr,"failed to read signal fd buffer\n");
    goto done;
  }

  switch(info.ssi_signo) {
    case SIGALRM: 
      if ((++cfg.ticks % 2) == 0) periodic_work();
      alarm(1); 
      break;
    default: 
      fprintf(stderr,"got signal %d\n", info.ssi_signo);  
      goto done;
      break;
  }

 rc = 0;

 done:
  return rc;
}

/* this toggles terminal settings so we read keystrokes on
 * stdin immediately (by negating canon and echo flags). 
 *
 * Undo at exit, or user's terminal will appear dead!!
 */
int want_keys(int want_keystrokes) {
  int rc = -1;
  struct termios t;

  if (isatty(STDIN_FILENO) == 0) return 0;

  if (tcgetattr(STDIN_FILENO, &t) < 0) {
    fprintf(stderr,"tcgetattr: %s\n", strerror(errno));
    goto done;
  }

  if (want_keystrokes) t.c_lflag &= ~(ICANON|ECHO);
  else                 t.c_lflag |=  (ICANON|ECHO);

  if (tcsetattr(STDIN_FILENO, TCSANOW, &t) < 0) {
    fprintf(stderr,"tcsetattr: %s\n", strerror(errno));
    goto done;
  }
  rc = 0;
 done:
  return rc;
}

int keyclicks=0;
int handle_stdin(void) {
  int rc= -1, bc;
  char c;

  bc = read(STDIN_FILENO, &c, sizeof(c));
  if (bc <= 0) goto done;

  if (c == 'q') goto done; /* quit */ 
  else fprintf(stderr, "%u\n", ++keyclicks);
  rc = 0;

 done:
  return rc;
}

int main(int argc, char *argv[]) {
  int opt, n;
  cfg.prog = argv[0];
  struct epoll_event ev;

  while ( (opt=getopt(argc,argv,"vh")) != -1) {
    switch(opt) {
      case 'v': cfg.verbose++; break;
      case 'h': default: usage(); break;
    }
  }

  /* block all signals. we take signals synchronously via signalfd */
  sigset_t all;
  sigfillset(&all);
  sigprocmask(SIG_SETMASK,&all,NULL);

  /* a few signals we'll accept via our signalfd */
  sigset_t sw;
  sigemptyset(&sw);
  for(n=0; n < sizeof(sigs)/sizeof(*sigs); n++) sigaddset(&sw, sigs[n]);

  /* create the signalfd for receiving signals */
  cfg.signal_fd = signalfd(-1, &sw, 0);
  if (cfg.signal_fd == -1) {
    fprintf(stderr,"signalfd: %s\n", strerror(errno));
    goto done;
  }

  /* unbuffer keypresses from terminal */
  if (want_keys(1) < 0) goto done;

  /* set up the epoll instance */
  cfg.epoll_fd = epoll_create(1); 
  if (cfg.epoll_fd == -1) {
    fprintf(stderr,"epoll: %s\n", strerror(errno));
    goto done;
  }

  /* add descriptors of interest */
  if (new_epoll(EPOLLIN, cfg.signal_fd))   goto done; // signal socket
  if (new_epoll(EPOLLIN, STDIN_FILENO)) goto done; // keyboard

  fprintf(stderr,"starting... press keys. press q to exit\n");

  alarm(1);
  while (epoll_wait(cfg.epoll_fd, &ev, 1, -1) > 0) {
    if      (ev.data.fd == cfg.signal_fd) { if (handle_signal() < 0) goto done;}
    else if (ev.data.fd == STDIN_FILENO)  { if (handle_stdin()  < 0) goto done;}
  }

done:
  if (cfg.epoll_fd != -1) close(cfg.epoll_fd);
  if (cfg.signal_fd != -1) close(cfg.signal_fd);
  want_keys(0); /* restore terminal */
  return 0;
}
