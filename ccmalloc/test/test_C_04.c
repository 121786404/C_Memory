#include <stdlib.h>

int main()
{
  char * a = (char*) malloc(15);
  free(a);
  a[0]=1;
  free(a);
  exit(0);
  return 0;
}
