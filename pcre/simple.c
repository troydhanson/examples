#include <pcre.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  int i,rc;

  /* for pcre to report compilation errors back to us */
  const char *err;
  int off;

  char *regex = "hello,?\\sworld!?";

  pcre *re = pcre_compile(regex, 0, &err, &off, NULL);
  if (re == NULL) {
    printf("error %s in pattern %s at offset %u\n", err, regex, off);
    exit(-1);
  }

  char *tests[] = { "hello, world!", "hello world!", "hello world" }; 

  /* now we coule loop over some input and test the regex */
  for(i=0; i < sizeof(tests)/sizeof(*tests); i++) {
    rc = pcre_exec(re, NULL, tests[i], strlen(tests[i]), 0, 0, NULL, 0);
    if (rc >= 0) {
      printf("%s matches %s\n", tests[i], regex);
    }
  }
}
