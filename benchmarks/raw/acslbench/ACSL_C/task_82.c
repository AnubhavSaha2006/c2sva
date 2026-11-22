/*
 * Task 82 Natural Language:
 * Write an ACSL-annotated C function that finds the last occurrence position of element `elem` in
 * array `arr` of length `len`. Returns -1 if not found, otherwise ensures the returned position is
 * valid and all subsequent elements don't match `elem`.
 */

#include <stddef.h>

/*@
  requires \valid_read(arr + (0..len-1));
  requires len > 0;
  assigns \nothing;
  ensures (\result == -1) || (0 <= \result < (int)len && arr[\result] == elem);
  ensures (\forall integer i; \result < i < (int)len ==> arr[i] != elem);
*/
int LastPosition(const int arr[], size_t len, int elem) {
    int pos = -1;

    /*@
      loop invariant 0 <= i <= len;
      loop invariant pos == -1 || (0 <= pos < (int)i && arr[pos] == elem);
      loop invariant (\forall integer j; 0 <= j < (int)i && arr[j] == elem ==> j <= pos);
      loop assigns i, pos;
      loop variant len - i;
    */
    for (size_t i = 0; i < len; ++i) {
        if (arr[i] == elem) {
            pos = (int)i;
        }
    }

    return pos;
}