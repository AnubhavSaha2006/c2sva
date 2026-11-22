/*
 * Task 57 Natural Language:
 * Write an ACSL-annotated C function that performs pairwise addition on array `a` of size `n`,
 * storing results in `result`. The function requires non-empty input array and valid output space,
 * computes each result[i] as a[i] + a[i+1], and returns the count of operations (n-1).
 */

#include <stdlib.h>
#include <assert.h>

/*@
  requires n > 0;
  requires \valid(a + (0..n-1));
  requires \valid(result + (0..n-2));
  assigns result[0..n-2];
  ensures \forall integer i; 0 <= i < n-1 ==> result[i] == a[i] + a[i+1];
  ensures \result == n-1;
*/
size_t PairwiseAddition(const int a[], size_t n, int result[]) {
    /*@
      loop invariant 0 <= i <= n-1;
      loop invariant \forall integer k; 0 <= k < i ==> result[k] == a[k] + a[k+1];
      loop assigns i, result[0..n-2];
      loop variant n-1 - i;
    */
    for (size_t i = 0; i < n-1; ++i) {
        result[i] = a[i] + a[i+1];
    }
    return n-1;
}