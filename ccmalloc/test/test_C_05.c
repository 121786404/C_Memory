#include <stdlib.h>
#include <string.h>

#define S "Hello World"

int main()
{
  char * s = strcpy((char*) malloc(strlen(S)), S);
  free(s);
  exit(0);
  return 0;
}
