/*
 * Task 332 Natural Language:
 * Write an ACSL-annotated C function that increments an integer `x0`. The function requires that
 * `x0` is less than `INT_MAX` and ensures the result is greater than `x0`.
 */

#include <limits.h>
/*@
requires (x0<INT_MAX);
assigns \nothing;
ensures (\result>x0);
*/
int inc(int  x0) {
  int x2 = x0 + 1;
  return x2;
}
