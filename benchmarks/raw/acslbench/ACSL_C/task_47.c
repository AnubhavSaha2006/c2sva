/*
 * Task 47 Natural Language:
 * Write an ACSL-annotated C function that appends array `a` (of length `a_len`) to sequence `s`
 * (of length `s_len`), storing the result in array `r`. The function requires valid input arrays
 * and ensures `r` contains `s` followed by `a`, returning the combined length (`s_len + a_len`).
 */

#include <stdlib.h>
#include <assert.h>

/*@
  requires a_len > 0;
  requires \valid(s + (0..s_len-1));
  requires \valid(a + (0..a_len-1));
  requires \valid(r + (0..s_len + a_len - 1));
  ensures \forall integer i; 0 <= i < s_len ==> r[i] == s[i];
  ensures \forall integer i; 0 <= i < a_len ==> r[s_len + i] == a[i];
  ensures \result == s_len + a_len;
*/
size_t AppendArrayToSeq(int* r, const int* s, size_t s_len, const int* a, size_t a_len) {
    /*@
      loop invariant 0 <= i <= a_len;
      loop invariant \forall integer j; 0 <= j < s_len ==> r[j] == s[j];
      loop invariant \forall integer j; 0 <= j < i ==> r[s_len + j] == a[j];
      loop assigns i, r[s_len..s_len + a_len - 1];
      loop variant a_len - i;
    */
    for (size_t i = 0; i < a_len; ++i) {
        r[s_len + i] = a[i];
    }
    return s_len + a_len;
}