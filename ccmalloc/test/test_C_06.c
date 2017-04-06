#include <stdlib.h>

int main()
{
  char * s = (char*) malloc(4);
  s[-1]='A';
  free(s);
  exit(0);
  return 0;
}
