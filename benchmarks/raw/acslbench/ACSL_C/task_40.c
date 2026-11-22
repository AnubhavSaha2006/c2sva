/*
 * Task 40 Natural Language:
 * Write an ACSL-annotated C function that calculates the sum of elements in array `a` between
 * indices `start` (inclusive) and `end` (exclusive). The function requires valid indices within
 * array bounds (0 ≤ start ≤ end ≤ len).  

Write an ACSL-annotated recursive C function `sumTo`
 * that computes the sum of array `a` from index `start` to `end-1`. Returns 0 if `start == end`,
 * otherwise recursively sums elements. Requires valid bounds (0 ≤ start ≤ end ≤ len).
 */

#include <stdio.h>
#include <assert.h>

/*@
  requires \valid(a + (0..len-1));
  requires len > 0;
  requires 0 <= start <= end <= len;
  assigns \nothing;
  ensures \result == \sum(start, end-1, \lambda integer k; a[k]);
*/
int SumInRange(const int a[], int len, int start, int end) {
    int sum = 0;

    /*@
      loop invariant start <= i <= end;
      loop invariant sum == \sum(start, i-1, \lambda integer k; a[k]);
      loop assigns i, sum;
      loop variant end - i;
    */
    for (int i = start; i < end; ++i) {
        sum += a[i];
    }

    return sum;
}

/*@
  requires \valid(a + (0..len-1));
  requires len > 0;
  requires 0 <= start <= end <= len;
  assigns \nothing;
  ensures \result == \sum(start, end-1, \lambda integer k; a[k]);
  ensures start == end ==> \result == 0;
*/
int sumTo(const int a[], int len, int start, int end) {
    if (start == end) {
        return 0;
    } else {
        return sumTo(a, len, start, end-1) + a[end-1];
    }
}