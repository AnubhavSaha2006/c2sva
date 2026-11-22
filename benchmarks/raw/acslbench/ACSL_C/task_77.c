/*
 * Task 77 Natural Language:
 * Write an ACSL-annotated C function that performs element-wise addition of two integer arrays (a
 * and b) and stores the results in a third array (result). The function requires equal array
 * lengths (len) and valid memory ranges for all three arrays. Each result[i] will contain the sum
 * of a[i] + b[i].
 */

#include <stdlib.h>
#include <assert.h>

/*@
  requires len >= 0;
  requires \valid(a + (0..len-1));
  requires \valid(b + (0..len-1));
  requires \valid(result + (0..len-1));
  assigns result[0..len-1];
  ensures \forall integer i; 0 <= i < len ==> result[i] == a[i] + b[i];
*/
void AddLists(const int a[], const int b[], int result[], size_t len) {
    /*@
      loop invariant 0 <= i <= len;
      loop invariant \forall integer k; 0 <= k < i ==> result[k] == a[k] + b[k];
      loop assigns i, result[0..len-1];
      loop variant len - i;
    */
    for (size_t i = 0; i < len; ++i) {
        result[i] = a[i] + b[i];
    }
}