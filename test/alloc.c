#include <stdlib.h>

void alloc(int **p, int a, int b, int c, int d) {
  *p = calloc(1, sizeof(int));
  **p = a; *(*p+1) = b;*(*p+2) = c;*(*p+3) = d;
}
