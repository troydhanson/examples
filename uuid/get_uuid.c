#include <stdio.h>
#include <uuid/uuid.h>
int main() {
  uuid_t u;
  uuid_generate(u);
  printf("uuid is %d bytes\n", (int)(sizeof(u)));
  
  int i;
  for(i=0; i<16;i++) {
    printf("%.2x%c", (unsigned)(u[i]), (i<15)?'-':'\n');
  }
}
