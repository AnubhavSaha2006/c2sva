/*
 * Task 483 Natural Language:
 * Write an ACSL-annotated C function that performs integer division of a by b. The function
 * requires that the divisor b is not zero and that the division does not cause integer overflow
 * (specifically, a cannot be INT_MIN when b is -1). The function returns the quotient a/b without
 * any side effects.
 */

#include "limits.h"

/*@ requires b != 0;
  @ requires !(a == INT_MIN && b == -1);
  @ ensures \result == a/b;
  @ assigns \nothing;
  @*/
int div(int a, int b) {
  return a / b;
}