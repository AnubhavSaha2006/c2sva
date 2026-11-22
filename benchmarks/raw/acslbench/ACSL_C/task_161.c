/*
 * Task 161 Natural Language:
 * Write an ACSL-annotated C function that returns the integer 1. The function has no parameters
 * and ensures the result is always equal to 1.
 */

#include<assert.h>

/*@
  @ ensures \result == 1;
  @
 */
int gimmeOne() {
  return 1;
}

int main() {
  int x = gimmeOne();

  assert(x == 1);
}
