#include <sys/prctl.h>
#include <errno.h>
#include <stdio.h>

/* set name of calling thread; see prctl(2) */

int main(int argc, char *argv[]) {
  int rc = -1;
  char c;

  char *name = (argc > 1) ? argv[1] : "twix";
  if (prctl(PR_SET_NAME, name) < 0) {
    fprintf(stderr, "prctl: %s\n", strerror(errno));
    goto done;
  }

  fprintf(stderr, "pid %d thread name set to %s\n", getpid(), name);

  fprintf(stderr, "compare output of:\n\n");
  fprintf(stderr, "  ps -a                        (shows new name)\n");
  fprintf(stderr, "  top                          (shows new name)\n");
  fprintf(stderr, "  pstree                       (shows new name)\n");
  fprintf(stderr, "  grep Name /proc/<pid>/status (shows new name)\n");
  fprintf(stderr, "  ps -ax                       (shows old name)\n");
  fprintf(stderr, "  cat /proc/<pid>/cmdline      (shows old name)\n");
  fprintf(stderr, "\n");

  fprintf(stderr, "press <enter> to quit: ");
  read(0, &c, 1);
  rc = 0;

 done:
  return rc;
}
