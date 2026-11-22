/*
 * Task 378 Natural Language:
 * Write an ACSL-annotated C function that computes the absolute value of integer x. The function
 * requires that x is greater than INT_MIN to prevent undefined behavior. It assigns to external
 * variable a and ensures the result equals x when x is non-negative, or -x when x is negative.
 */


// Side effects can be controlled on an individual basis.

#include <limits.h>

extern int a;

/*@
    requires x > INT_MIN;

    assigns a; // allow assignment to a (but only to a).

    ensures 0 <= x ==> \result ==  x;
    ensures 0 >  x ==> \result == -x;
*/
int abs_int(int x)
{
  a = x;
  return (x >= 0) ? x : -x;
}



