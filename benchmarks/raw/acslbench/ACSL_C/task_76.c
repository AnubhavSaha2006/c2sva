/*
 * Task 76 Natural Language:
 * Write an ACSL-annotated C function that reverses the elements of an integer array `a` of length
 * `n`. The function requires a valid non-empty array and ensures each element at position `k`
 * equals the original element at position `n-1-k`.
 */

#include <stdio.h>
#include <assert.h>

/*@
  requires \valid(a + (0..n-1));
  requires n > 0;
  assigns a[0..n-1];
  ensures \forall integer k; 0 <= k < n ==> a[k] == \old(a[n-1-k]);
*/
void Reverse(int a[], size_t n) {
    size_t i = 0;

    /*@
      loop invariant 0 <= i <= n/2;
      loop assigns i, a[0..n-1];
      loop variant n/2 - i;
    */
    while (i < n / 2) {
        int temp = a[i];
        a[i] = a[n-1-i];
        a[n-1-i] = temp;
        ++i;
    }
}

