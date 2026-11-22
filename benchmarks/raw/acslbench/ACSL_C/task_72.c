/*
 * Task 72 Natural Language:
 * Write an ACSL-annotated C function that finds the smallest element in an integer array s of
 * length len. The function requires a non-empty array and ensures the result is both a minimum
 * value and exists in the array.
 */

#include <stdio.h>
#include <assert.h>

/*@
  requires len > 0;
  assigns \nothing;
  ensures \forall integer i; 0 <= i < len ==> \result <= s[i];
  ensures \exists integer i; 0 <= i < len && \result == s[i];
*/
int FindSmallest(const int s[], size_t len) {
    int min = s[0];

    /*@
      loop invariant 1 <= i <= len;
      loop invariant \forall integer k; 0 <= k < i ==> min <= s[k];
      loop invariant \exists integer k; 0 <= k < i && min == s[k];
      loop assigns i, min;
      loop variant len - i;
    */
    for (size_t i = 1; i < len; ++i) {
        if (s[i] < min) {
            min = s[i];
        }
    }

    return min;
}