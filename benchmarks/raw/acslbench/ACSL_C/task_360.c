/*
 * Task 360 Natural Language:
 * Write an ACSL-annotated C function that computes the absolute value of an integer `a`. The
 * function requires that `a` is not equal to `INT_MIN` and ensures the result is non-negative and
 * equals either `a` or `-a`.
 */

// pour INT_MIN
#include <limits.h>

// valeur absolue
/*@
 requires a != INT_MIN;
 ensures \result >= 0;
 ensures \result == a || \result == -a;
 */
int abs(int a)
{
	return (a > 0) ? a : -a;
}
