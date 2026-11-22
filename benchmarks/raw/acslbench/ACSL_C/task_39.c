/*
 * Task 39 Natural Language:
 * Write an ACSL-annotated C function that returns the closest smaller integer (n-1) for a given
 * positive input n. The function requires that n > 0 and ensures the result equals n-1.
 */

#include <stdio.h>
#include <assert.h>

/*@
  requires n > 0;
  assigns \nothing;
  ensures \result == n - 1;
*/
int ClosestSmaller(int n) {
    int m = n - 1;
    return m;
}