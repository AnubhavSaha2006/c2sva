/*
 * Task 60 Natural Language:
 * Write an ACSL-annotated C function that checks if an integer array `a` of length `len` is sorted
 * in non-decreasing order. The function returns true if all adjacent elements satisfy `a[i] <=
 * a[i+1]`, false otherwise. Requires the array length `len` to be greater than 0.
 */

#include <stdbool.h>
#include <stddef.h>

/*@
  requires len > 0;
  assigns \nothing;
  ensures \result <==> (\forall integer i, j; 0 <= i < j < len ==> a[i] <= a[j]);
*/
bool IsSorted(const int a[], size_t len) {
    bool sorted = true;

    /*@
      loop invariant 0 <= i < len;
      loop invariant sorted <==> (\forall integer k; 0 <= k < i ==> a[k] <= a[k+1]);
      loop assigns i, sorted;
      loop variant len - 1 - i;
    */
    for (size_t i = 0; i < len - 1; ++i) {
        if (a[i] > a[i + 1]) {
            sorted = false;
            break;
        }
    }

    return sorted;
}