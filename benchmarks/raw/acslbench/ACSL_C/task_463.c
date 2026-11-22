/*
 * Task 463 Natural Language:
 * Write an ACSL-annotated C function that computes the absolute value of an integer x. The
 * function requires that x is greater than INT_MIN to avoid undefined behavior, and ensures the
 * result is non-negative and equals x when x is non-negative, or equals -x when x is negative.
 */

#include <limits.h>

/*@
    requires x > INT_MIN;

    assigns \nothing; // forbid any side effects

    ensures 0 <= x ==> \result ==  x;
    ensures 0 >  x ==> \result == -x;
*/
int abs_int(int x)
{
  return (x >= 0) ? x : -x;
}
