/*
 * Task 595 Natural Language:
 * Write an ACSL-annotated C function that computes the absolute value of an integer x. The
 * function requires that x is not equal to INT_MIN and ensures that the result is non-negative: if
 * x is positive or zero, the result equals x; if x is negative, the result equals -x.
 */

#include <limits.h>

/*@
  requires x != INT_MIN;
  assigns \nothing;

  ensures x >= 0 ==> \result ==  x;
  ensures x <  0 ==> \result == -x;
*/
int abs(int x) {
  if (x >= 0) {
    return x;
  } else {
    return -x;
  }
}
