/*
 * Task 65 Natural Language:
 * Write an ACSL-annotated C function that swaps the first and last elements of an integer array.
 * The function requires a non-empty array (len > 0) and modifies elements a[0] and a[len-1],
 * leaving other elements unchanged.
 */

#include <stdio.h>
#include <assert.h>

/*@
  requires len > 0;
  requires \valid(a + (0..len-1));
  assigns a[0], a[len-1];
  ensures a[0] == \old(a[len-1]);
  ensures a[len-1] == \old(a[0]);
  ensures \forall integer k; 1 <= k < len-1 ==> a[k] == \old(a[k]);
*/
void SwapFirstAndLast(int a[], size_t len) {
    int temp = a[0];
    a[0] = a[len-1];
    a[len-1] = temp;
}