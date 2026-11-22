/*
 * Task 58 Natural Language:
 * Write an ACSL-annotated C function `sumNegativesTo` that recursively sums all negative elements
 * in array `a` up to index `n-1`. Requires `n` to be within bounds (`n <= len`).

Write an ACSL-
 * annotated C function `SumOfNegatives` that iteratively computes the sum of all negative elements
 * in array `a` of length `len`. Works for any valid array length (`len >= 0`).
 */

#include <stddef.h>
#include <assert.h>

/*@
  requires n <= len;
  assigns \nothing;
  ensures \result == \sum(0, n-1, \lambda integer k; (a[k] < 0) ? a[k] : 0);
*/
int sumNegativesTo(const int a[], size_t len, size_t n) {
    if (n == 0) {
        return 0;
    } else {
        int lastElement = a[n - 1];
        int sum = sumNegativesTo(a, len, n - 1);
        if (lastElement < 0) {
            return sum + lastElement;
        } else {
            return sum;
        }
    }
}

/*@
  requires len >= 0;
  assigns \nothing;
  ensures \result == \sum(0, len-1, \lambda integer k; (a[k] < 0) ? a[k] : 0);
*/
int SumOfNegatives(const int a[], size_t len) {
    int result = 0;

    /*@
      loop invariant 0 <= i <= len;
      loop invariant result == \sum(0, i-1, \lambda integer k; (a[k] < 0) ? a[k] : 0);
      loop assigns i, result;
      loop variant len - i;
    */
    for (size_t i = 0; i < len; ++i) {
        if (a[i] < 0) {
            result += a[i];
        }
    }

    return result;
}