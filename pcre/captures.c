#include <pcre.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define OVECSZ 30 /* must be multiple of 3 */

/* sample usage: ./captures 'abc(\d+)(_(\d+))?' abc123_999 */

int main(int argc, char *argv[]) {
  int i,rc,j;

  if (argc < 3) {
      fprintf(stderr, "usage: <regex> <testpat1> ...\n");
      exit(-1);
  }

  const char *err; int off, ovec[OVECSZ];

  pcre *re = pcre_compile(argv[1], 0, &err, &off, NULL);
  if (re == NULL) {
      fprintf(stderr, "error in regex %s: %s (offset %u)\n", argv[1], err, off);
      exit(-1);
  }

  for(i=2; i < argc; i++) {
    char *s = argv[i];
    printf("testing %s:\n",s);
    if ( (rc=pcre_exec(re, NULL, s, strlen(s), 0, 0, ovec, OVECSZ)) > 0) {
      for(j=0; j < rc*2; j+=2) {
        printf(" $%u matched %.*s\n", j/2, ovec[j+1]-ovec[j], &s[ovec[j]]);
      }
    }
    printf("\n");
  }
}
