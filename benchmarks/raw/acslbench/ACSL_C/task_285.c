/*
 * Task 285 Natural Language:
 * Write an ACSL-annotated C function that computes the absolute value of an integer x. The
 * function requires that x is greater than INT_MIN and ensures the result is non-negative,
 * equaling x if x is non-negative and -x otherwise.
 */


#include <limits.h>

/*@
    requires x > INT_MIN;

    ensures 0 <= x ==> \result ==  x;
    ensures 0 >  x ==> \result == -x;
*/
int abs_int(int x)
{
  return (x >= 0) ? x : -x;
}

