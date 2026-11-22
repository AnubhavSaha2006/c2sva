/*
 * Task 73 Natural Language:
 * Write an ACSL-annotated C function that swaps the first and last elements of an integer array.
 * The function requires a non-empty array (n > 0) and ensures all middle elements remain unchanged
 * while swapping positions of a[0] and a[n-1].
 */

#include <stdio.h>
#include <assert.h>

/*@
  requires n > 0;
  requires \valid(a + (0..n-1));
  assigns a[0], a[n-1];
  ensures a[0] == \old(a[n-1]);
  ensures a[n-1] == \old(a[0]);
  ensures \forall integer k; 1 <= k < n-1 ==> a[k] == \old(a[k]);
*/
void SwapFirstAndLast(int a[], size_t n) {
    int old_first = a[0];
    int old_last = a[n-1];

    int tmp = a[0];
    a[0] = a[n-1];
    a[n-1] = tmp;
}