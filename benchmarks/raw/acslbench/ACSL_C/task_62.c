/*
 * Task 62 Natural Language:
 * Write an ACSL-annotated C function that splits array 'l' at position 'n' and appends the first
 * part to the end, storing the result in array 'r'. The function requires valid indices (0 ≤ n <
 * len) and equal-length arrays l and r. The result r contains elements rotated by n positions.
 */

#include <stdlib.h>

/*@
  requires n >= 0 && n < len;
  requires \valid(l + (0..len-1));
  requires \valid(r + (0..len-1));
  assigns r[0..len-1];
  ensures \forall integer i; 0 <= i < len ==> r[i] == l[(i + n) % len];
*/
void SplitAndAppend(const int l[], size_t len, size_t n, int r[]) {
    /*@
      loop invariant 0 <= i <= len;
      loop invariant \forall integer j; 0 <= j < i ==> r[j] == l[(j + n) % len];
      loop assigns i, r[0..len-1];
      loop variant len - i;
    */
    for (size_t i = 0; i < len; ++i) {
        r[i] = l[(i + n) % len];
    }
}