/*
 * Task 36 Natural Language:
 * Write an ACSL-annotated C function that returns the maximum difference between any two elements
 * in array `a` of length `len`. The function requires the array to have at least 2 elements and
 * guarantees the result is the actual maximum difference found in the array.
 */

#include <stdio.h>
#include <limits.h>

/*@
  requires len > 1;
  assigns \nothing;
  ensures \forall integer i, j; 0 <= i && i < len && 0 <= j && j < len ==> a[i] - a[j] <= \result;
  ensures \exists integer i, j; 0 <= i && i < len && 0 <= j && j < len && a[i] - a[j] == \result;
*/
int MaxDifference(const int a[], size_t len) {
    int minVal = a[0];
    int maxVal = a[0];

    /*@
      loop invariant 1 <= i <= len;
      loop invariant \exists integer k; 0 <= k < i && a[k] == minVal;
      loop invariant \exists integer k; 0 <= k < i && a[k] == maxVal;
      loop invariant minVal <= maxVal;
      loop assigns i, minVal, maxVal;
      loop variant len - i;
    */
    for (size_t i = 1; i < len; ++i) {
        if (a[i] < minVal) {
            minVal = a[i];
        } else if (a[i] > maxVal) {
            maxVal = a[i];
        }
    }

    return maxVal - minVal;
}