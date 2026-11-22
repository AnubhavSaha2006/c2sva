/*
 * Task 208 Natural Language:
 * Write an ACSL-annotated C program that tests E-ACSL instrumentation options. The program
 * includes several functions with and without instrumentation, using pointers, goto statements,
 * and variadic arguments. It requires valid pointers for certain functions and ensures variables
 * are initialized before use.
 */


#include <stdarg.h>

int uninstrument1(int *p) {
  *p = 0;
  return 0;
}

/*@ requires \valid(p); */
int uninstrument2(int *p) {
  { int *q = p;
    *p = 0;
    goto L;
  }
 L:
  return 0;
}

int instrument1(int *p) {
  *p = 0;
  return 0;
}

/*@ requires \valid(p); */
int instrument2(int *p) {
  { int *q = p;
    *p = 0;
    goto L;
  }
 L:
  return 0;
}

/* test combination of -e-acsl-instrument and -variadic-no-translation;
   see gitlab's issue #88 */
int vol(int n, ...) {
  va_list vl;
  va_start(vl, n);
  int r = va_arg(vl, int);
  va_end(vl);
  return 1;
}

int main(void) {
  int x;
  int y = 0;
  instrument1(&x);
  uninstrument1(&x);
  instrument2(&x);
  uninstrument2(&x);
  /*@ assert \initialized(&x); */
  /*@ assert \initialized(&y); */
  return vol(6, 1);
}
