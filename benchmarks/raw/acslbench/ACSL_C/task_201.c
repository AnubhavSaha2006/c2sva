/*
 * Task 201 Natural Language:
 * Write an ACSL-annotated C program containing two functions, `add` and `incr`, which are intended
 * to manipulate integer pointers. The `add` function takes a pointer `x` and an integer `val`,
 * requires `val >= 0`, and ensures the result is `val + 1`. The `incr` function takes a pointer
 * `x`, requires it to be valid and point to a value `>= 0`, and ensures the pointed value equals
 * the result minus one. The program contains errors: the `add` function requires a valid pointer
 * `x`, and the `incr` function fails to free allocated memory for `y`. The main function
 * incorrectly calls `incr` with an invalid pointer.
 */


#include <stdlib.h>
/*@ 
  @ requires val >= 0;
  @ ensures \result == val + 1;
  @
*/
int add(int * x, int val) 
{
  *x = val + 1;
  return *x;
}

/*@  requires *x >= 0;
  @  requires \valid(x);
  @  ensures *x == \result - 1;
  @  ensures \valid(x);
  @*/
int incr(int * x) 
{ 
  int * y;
  int ret;

  y = (int*)malloc(sizeof(int));
  *y = *x;
  ret = add(x, *y);
  return ret;
}

int main() {
  incr((void *)-1);
  return 0;
}
