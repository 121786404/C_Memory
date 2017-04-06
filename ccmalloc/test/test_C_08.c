#include <stdlib.h>

int
main()
{
  char * a = (char*) malloc(100);
  char * b = (char*) malloc(100);
  char * c = (char*) malloc(100);
  char * d = (char*) malloc(100);
  char * e = (char*) malloc(100);
  char * f = (char*) malloc(100);
  char * g = (char*) malloc(100);
  char * h = (char*) malloc(100);
  char * i = (char*) malloc(100);
  char * j = (char*) malloc(100);
  char * k = (char*) malloc(100);
  char * l = (char*) malloc(100);
  char * m = (char*) malloc(100);
  char * n = (char*) malloc(100);
  char * o = (char*) malloc(100);
  char * p = (char*) malloc(100);
  char * q = (char*) malloc(100);

  free(q);
  free(p);
  free(o);
  free(n);
  free(m);

  a[120] = 2;			/* BOOM */

  free(l);
  free(k);
  free(j);
  free(i);
  free(h);

  g[-9] = 2;			/* BOOM */

  free(g);
  free(f);
  free(e);
  free(d);
  free(c);
  free(b);
  free(a);

  exit(0);
  return 0;
}
