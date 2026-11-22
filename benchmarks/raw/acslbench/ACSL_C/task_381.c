/*
 * Task 381 Natural Language:
 * Write an ACSL-annotated C function that swaps the first two elements of an integer array. The
 * function requires a valid pointer to an array with at least two elements and ensures the values
 * at indices 0 and 1 are exchanged.
 */

#include <limits.h>
/*@
requires \valid(x0+(0..1));
ensures ((x0[0]==\old(x0[1])) &&
(x0[1]==\old(x0[0])));
*/
void array_swap(int  * x0) {
  int x2 = x0[0];
  int x3 = x0[1];
  x0[0] = x3;
  x0[1] = x2;
}
