/*
 * Task 310 Natural Language:
 * Write an ACSL-annotated C program that demonstrates function calls with pointer parameters and
 * dynamic memory allocation. The function `f` takes two integer pointers `x` and `y`, sets the
 * value pointed by `y` to 1, and returns `x`. The `main` function allocates memory for two
 * integers, calls `f` with different pointer arguments, and ensures all memory accesses are valid.
 */


#include <stdlib.h>

//extern void *malloc(unsigned int size);

/*@ ensures \valid(\result); */
int *f(int *x, int *y) {
  *y = 1;
  return x;
}

int main() {
  int x = 0, *p, *q = malloc(sizeof(int)), *r = malloc(sizeof(int));
  p = f(&x, q);
  q = f(&x, r);
  return 0;
}
