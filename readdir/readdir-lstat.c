#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

char path[PATH_MAX];

int main(int argc, char *argv[]) {
  int rc = -1, i = 0;
  char *dirname, *type, *mode;
  struct dirent *dent;
  struct stat s;
  DIR *d = NULL;
  size_t dl, el;

  dirname = (argc > 1) ? argv[1] : ".";

  d = opendir(dirname);
  if (d == NULL) {
    fprintf(stderr, "opendir %s: %s\n", dirname, strerror(errno));
    goto done;
  }

  while ( (dent = readdir(d)) != NULL) {

    switch(dent->d_type) {
      case DT_BLK:     type = "DT_BLK";     break;
      case DT_CHR:     type = "DT_CHR";     break;
      case DT_DIR:     type = "DT_DIR";     break;
      case DT_FIFO:    type = "DT_FIFO";    break;
      case DT_LNK:     type = "DT_LNK";     break;
      case DT_REG:     type = "DT_REG";     break;
      case DT_SOCK:    type = "DT_SOCK";    break;
      case DT_UNKNOWN: type = "DT_UNKNOWN"; break;
      default:         type = "other";      break;
    }

    /* now to lstat this entry we need to formulate it into a path */
    dl = strlen(dirname);
    el = strlen(dent->d_name);
    if (dl+1+el+1 > sizeof(path)) goto done;
    memcpy(path, dirname, dl);
    path[dl] = '/';
    memcpy(&path[dl+1], dent->d_name, el+1);

    if (lstat(path, &s) < 0) {
      fprintf(stderr, "stat %s: %s\n", path, strerror(errno));
      continue;
    }

    mode = "unknown";
    if (S_ISREG(s.st_mode))  mode = "file";
    if (S_ISDIR(s.st_mode))  mode = "directory";
    if (S_ISCHR(s.st_mode))  mode = "chardev";
    if (S_ISBLK(s.st_mode))  mode = "blockdev";
    if (S_ISFIFO(s.st_mode)) mode = "fifo";
    if (S_ISLNK(s.st_mode))  mode = "symlink";
    if (S_ISSOCK(s.st_mode)) mode = "socket";

    printf(" %d: inode %lu type %s mode %s name %s\n", ++i,
      (unsigned long)dent->d_ino, type, mode, dent->d_name);
  }

  printf("%d entries found.\n", i);
  rc = 0;

 done:
  if (d) closedir(d);
  return rc;
}
