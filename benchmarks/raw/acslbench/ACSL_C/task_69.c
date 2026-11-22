/*
 * Task 69 Natural Language:
 * Write an ACSL-annotated C function that removes the element at position k from array s and
 * returns a new array. The function requires valid indices (0 ≤ k < size) and ensures the new
 * array contains all elements except s[k], with the remaining elements properly shifted. The
 * result array will have size-1 elements.
 */

#include <stdlib.h>
#include <assert.h>

/*@
  requires \valid(s+(0..size-1));
  requires 0 <= k < size;
  ensures \result != NULL;
  ensures \forall integer i; 0 <= i < k ==> \result[i] == s[i];
  ensures \forall integer i; k <= i < size-1 ==> \result[i] == s[i+1];
  ensures \result[size-1] == 0;
*/
int* RemoveElement(const int s[], size_t size, size_t k) {
    int* v = (int*)malloc((size-1)*sizeof(int));
    size_t i = 0;

    /*@
      loop invariant 0 <= i <= k;
      loop invariant \forall integer j; 0 <= j < i ==> v[j] == s[j];
      loop assigns i, v[0..k-1];
      loop variant k - i;
    */
    while (i < k) {
        v[i] = s[i];
        i++;
    }

    /*@
      loop invariant k <= i <= size-1;
      loop invariant \forall integer j; 0 <= j < k ==> v[j] == s[j];
      loop invariant \forall integer j; k <= j < i ==> v[j] == s[j+1];
      loop assigns i, v[k..size-2];
      loop variant size-1 - i;
    */
    while (i < size-1) {
        v[i] = s[i+1];
        i++;
    }

    return v;
}