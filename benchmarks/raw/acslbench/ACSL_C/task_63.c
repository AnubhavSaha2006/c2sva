/*
 * Task 63 Natural Language:
 * Write an ACSL-annotated C function that creates a new integer array (s) containing a copy of all
 * elements from the source array (a) with given length (len). The function requires a valid non-
 * empty input array and ensures the output array exactly matches the input elements.
 */

#include <stdlib.h>
#include <assert.h>

/*@
  requires len > 0;
  requires \valid(a + (0..len-1));
  assigns \nothing;
  ensures \result != NULL;
  ensures \forall integer i; 0 <= i < len ==> \result[i] == a[i];
*/
int* ArrayToSeq(const int a[], size_t len) {
    int* s = (int*)malloc(len * sizeof(int));
    
    /*@
      loop invariant 0 <= i <= len;
      loop invariant \forall integer j; 0 <= j < i ==> s[j] == a[j];
      loop assigns i, s[0..len-1];
      loop variant len - i;
    */
    for (size_t i = 0; i < len; ++i) {
        s[i] = a[i];
    }

    return s;
}