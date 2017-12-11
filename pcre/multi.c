#include <pcre.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* sample program that matches input against several regex's, 
 * extracts captured subpatterns. compile with -lpcre and test with:
 *
 * ./template one,two,three abc123
 *
 */

#define OVECLEN 30 /* must be multiple of three. */

typedef struct {
    char *regex;
    pcre *re;
    pcre_extra *x;
} regex_t;

regex_t rexes[] = {
  {.regex = "abc(\\d+)", },
  {.regex = "^(\\w+),(\\w+),(\\w+)$", },
};

int main(int argc, char *argv[]) {
  int i,j, ovec[OVECLEN], rc; regex_t *r;

  /* for pcre to report compilation errors back to us */
  const char *err;
  int off;

  for(i=0; i < sizeof(rexes)/sizeof(*rexes); i++) {
    r = &rexes[i];
    /* compile the regular expression */
    r->re = pcre_compile(r->regex, 0, &err, &off, NULL);
    if (r->re == NULL) {
        fprintf(stderr,"error %s in pattern %s at %u\n", err, r->regex, off);
        exit(-1);
    }

    /* optional: study the pattern */
    r->x = pcre_study(r->re, 0, &err);
    if (err) {
        fprintf(stderr,"study of %s failed: %s\n", r->regex, err);
        exit(-1);
    }
  }

  /* now we loop over some input and test the regex */
  for(i=0; i < argc; i++) {
    for(j=0; j < sizeof(rexes)/sizeof(*rexes); j++) {
      r = &rexes[j];
      rc = pcre_exec(r->re, r->x, argv[i], strlen(argv[i]), 0, 0, ovec, OVECLEN);
      if (rc >= 0) {
        printf("%s matches %s: first capture %.*s\n", argv[i], r->regex, 
          ovec[3]-ovec[2], &argv[i][ovec[2]]);
      }
    }
  }
}
