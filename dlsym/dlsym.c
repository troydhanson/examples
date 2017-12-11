#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

/*
 *  test this program using:
 *
 *    ./dlsym subject incr
 *    ./dlsym subject decr
 */
 
int   (*bfcn)(int); /* prototype of fcn to be called in lib */
 
int main(int argc, char *argv[]) {
  dlerror(); /* clear */

  if (argc < 2) {
    fprintf(stderr,"usage: %s <solib> [<symbol>]\n", argv[0]);
    exit(-1);
  }
  char *lib = argv[1];
  char *sym = (argc > 2) ? argv[2] : "incr";

  void *handle = dlopen(lib, RTLD_LAZY);
  if (!handle) {
    fprintf(stderr, "dlopen failed: %s\n", dlerror());
    exit(-1);
  }

  void *addr = dlsym(handle,sym);
  bfcn=addr;
  int rc=(*bfcn)(1);
  printf("rc is %d\n",rc);
  dlclose(handle);
}
