/* Test the new `read-dynlib-with-gdb' flag
 */

#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
#endif
void oops();

int main()
{
  char * a;

  a = (char*)malloc(13);
  free(a);
  a = strdup("oops");		/* if libc is compiled with `-g' this
  			         * should give you the strdup symbol 
				 */
  oops();

  exit(0);
  return 0;
}
