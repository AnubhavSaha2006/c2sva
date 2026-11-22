/*
 * Task 53 Natural Language:
 * Write an ACSL-annotated C function that calculates the directrix of a parabola given
 * coefficients a, h, and k. The function requires that a is non-zero and returns the directrix
 * value as k - (1.0/(4.0*a)).
 */

#include <stdio.h>
#include <assert.h>

/*@
  requires a != 0;
  assigns \nothing;
  ensures \result == k - (1.0 / (4.0 * a));
*/
double ParabolaDirectrix(int a, int h, int k) {
    double directrix = k - (1.0 / (4.0 * a));
    return directrix;
}