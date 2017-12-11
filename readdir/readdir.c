#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char *argv[]) {
  int rc = -1, i = 0;
  char *path, *type;
  struct dirent *dent;
  DIR *d = NULL;

  path = (argc > 1) ? argv[1] : ".";

  d = opendir(path);
  if (d == NULL) {
    fprintf(stderr, "opendir %s: %s\n", path, strerror(errno));
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

    printf(" %d: inode %lu type %s %s\n", ++i, (unsigned long)dent->d_ino, 
      type, dent->d_name);
  }

  printf("%d entries found.\n", i);
  rc = 0;

 done:
  if (d) closedir(d);
  return rc;
}
