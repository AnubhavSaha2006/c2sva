/*
 * Task 278 Natural Language:
 * Write an ACSL-annotated C function that increments an integer x by 1. The function requires that
 * x is less than INT_MAX and ensures the result is greater than x.
 */

#include <limits.h>
/*@ requires x < INT_MAX;
	ensures \result > x;
*/
int inc (int x) { return x+1; }

