/*
 * Task 83 Natural Language:
 * Write an ACSL-annotated C function `sumTo` that recursively computes the sum of the first `n`
 * elements in array `a` (where `n` must not exceed the array length `len`). Also includes an
 * iterative version `ArraySum` that calculates the total sum of all elements in array `a` of
 * length `len`.
 */

#include <stdio.h>
#include <assert.h>

/*@
  requires n <= len;
  assigns \nothing;
  ensures \result == \sum(0, n-1, \lambda integer k; a[k]);
*/
int sumTo(const int a[], size_t len, size_t n) {
    if (n == 0) {
        return 0;
    } else {
        return sumTo(a, len, n - 1) + a[n - 1];
    }
}

/*@
  requires len >= 0;
  assigns \nothing;
  ensures \result == \sum(0, len-1, \lambda integer k; a[k]);
*/
int ArraySum(const int a[], size_t len) {
    int result = 0;

    /*@
      loop invariant 0 <= i <= len;
      loop invariant result == \sum(0, i-1, \lambda integer k; a[k]);
      loop assigns i, result;
      loop variant len - i;
    */
    for (size_t i = 0; i < len; ++i) {
        result += a[i];
    }

    return result;
}