/*
 * Task 81 Natural Language:
 * Write an ACSL-annotated C function that counts how many positions in three arrays (a, b, c) have
 * identical values across all arrays. The function requires valid arrays of length n and returns
 * the count of matching triplets.
 */

#include <stdio.h>
#include <assert.h>

/*@
  requires \valid(a + (0..n-1)) && \valid(b + (0..n-1)) && \valid(c + (0..n-1));
  requires n >= 0;
  assigns \nothing;
  ensures \result == \sum(0, n-1, \lambda integer i; (a[i] == b[i] && b[i] == c[i]) ? 1 : 0);
*/
int CountIdenticalPositions(const int a[], const int b[], const int c[], size_t n) {
    size_t count = 0;

    /*@
      loop invariant 0 <= i <= n;
      loop invariant count == \sum(0, i-1, \lambda integer k; (a[k] == b[k] && b[k] == c[k]) ? 1 : 0);
      loop assigns i, count;
      loop variant n - i;
    */
    for (size_t i = 0; i < n; ++i) {
        if (a[i] == b[i] && b[i] == c[i]) {
            count++;
        }
    }

    return (int)count;
}