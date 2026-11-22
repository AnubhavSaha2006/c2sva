/*
 * Task 43 Natural Language:
 * Write an ACSL-annotated C function that counts the number of true values in a boolean array. The
 * function `CountTrue` takes an array `a` and its size, returning the total count of true
 * elements. The helper function `countTo` recursively counts true values up to index `n-1` in
 * array `a` of given size. Both require valid array bounds (n ≤ size for `countTo`, size > 0 for
 * `CountTrue`).
 */

#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

/*@
  requires n <= size;
  assigns \nothing;
  ensures \result == \sum(0, n-1, \lambda integer k; (a[k] == true) ? 1 : 0);
*/
int countTo(const bool a[], size_t size, size_t n) {
    if (n == 0) {
        return 0;
    } else {
        return countTo(a, size, n - 1) + (a[n - 1] ? 1 : 0);
    }
}

/*@
  requires size > 0;
  assigns \nothing;
  ensures \result == \sum(0, size-1, \lambda integer k; (a[k] == true) ? 1 : 0);
*/
int CountTrue(const bool a[], size_t size) {
    int result = 0;

    /*@
      loop invariant 0 <= i <= size;
      loop invariant result == \sum(0, i-1, \lambda integer k; (a[k] == true) ? 1 : 0);
      loop assigns i, result;
      loop variant size - i;
    */
    for (size_t i = 0; i < size; ++i) {
        if (a[i]) {
            result += 1;
        }
    }

    return result;
}