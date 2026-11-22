/*
 * Task 296 Natural Language:
 * Write an ACSL-annotated C function that computes the absolute value of an integer x. The
 * function requires that x is greater than INT_MIN to avoid overflow. It returns x if x is non-
 * negative, and returns -x if x is negative.
 */

#include <limits.h>

/*@ requires x > INT_MIN;
    assigns \nothing;

  behavior positive:
    assumes x >= 0;
    ensures \result == x;
  
  behavior negative:
    assumes x < 0;
    ensures \result == -x;

  complete behaviors;
  disjoint behaviors;
*/

int abs(int x) {
  
  if (x < 0)
    return -x;
  else if(x >= 0)
    return x;
}
