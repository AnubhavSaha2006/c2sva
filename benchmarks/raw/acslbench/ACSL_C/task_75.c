/*
 * Task 75 Natural Language:
 * Write an ACSL-annotated C function that finds the smallest missing non-negative integer in a
 * sorted array `s` of size `n`. The function requires the array to be sorted in non-decreasing
 * order and contain only non-negative numbers. The result is the smallest integer ≥ 0 not present
 * in the array.
 */

#include <stddef.h>
#include <assert.h>

/*@
  requires \forall integer i; 0 <= i < n-1 ==> s[i] <= s[i+1];
  requires \forall integer i; 0 <= i < n ==> s[i] >= 0;
  assigns \nothing;
  ensures 0 <= \result;
  ensures \forall integer i; 0 <= i < n ==> s[i] != \result;
  ensures \forall integer k; 0 <= k < \result ==> \exists integer i; 0 <= i < n && s[i] == k;
*/
int SmallestMissingNumber(const int s[], size_t n) {
    int v = 0;
    size_t i = 0;

    /*@
      loop invariant 0 <= i <= n;
      loop invariant 0 <= v <= (int)i;
      loop invariant \forall integer k; 0 <= k < v ==> \exists integer j; 0 <= j < i && s[j] == k;
      loop invariant \forall integer j; 0 <= j < i ==> s[j] != v;
      loop assigns i, v;
      loop variant n - i;
    */
    for (; i < n; ++i) {
        if (s[i] > v) {
            break;
        } else if (s[i] == v) {
            v += 1;
        }
    }

    return v;
}