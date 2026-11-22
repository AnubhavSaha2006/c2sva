/*
 * Task 64 Natural Language:
 * Write an ACSL-annotated C function that calculates the difference between the maximum and
 * minimum values in an integer array. The function requires a non-empty array (len > 0) and
 * returns the difference between the largest (maxVal) and smallest (minVal) elements found in
 * array a of length len.
 */

#include <stdio.h>
#include <limits.h>

/*@
  requires len > 0;
  assigns \nothing;
  ensures \result == (\max(0, len-1, \lambda integer i; a[i])) - 
                    (\min(0, len-1, \lambda integer i; a[i]));
*/
int DifferenceMinMax(const int a[], size_t len) {
    int minVal = a[0];
    int maxVal = a[0];

    /*@
      loop invariant 1 <= i <= len;
      loop invariant minVal == \min(0, i-1, \lambda integer k; a[k]);
      loop invariant maxVal == \max(0, i-1, \lambda integer k; a[k]);
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