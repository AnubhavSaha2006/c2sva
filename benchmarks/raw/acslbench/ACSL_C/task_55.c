/*
 * Task 55 Natural Language:
 * Write an ACSL-annotated C function that performs element-wise multiplication of two integer
 * arrays (a and b) and stores the results in a pre-allocated output array (result). The function
 * requires equal array lengths (n) and valid memory ranges for all arrays. Ensures each result[i]
 * equals a[i] * b[i].
 */

#include <stdlib.h>
#include <assert.h>

/*@
  requires n >= 0;
  requires \valid(a + (0..n-1));
  requires \valid(b + (0..n-1));
  requires \valid(result + (0..n-1));
  assigns result[0..n-1];
  ensures \forall integer i; 0 <= i < n ==> result[i] == a[i] * b[i];
*/
void MultiplyElements(const int a[], const int b[], int result[], size_t n) {
    /*@
      loop invariant 0 <= i <= n;
      loop invariant \forall integer k; 0 <= k < i ==> result[k] == a[k] * b[k];
      loop assigns i, result[0..n-1];
      loop variant n - i;
    */
    for (size_t i = 0; i < n; ++i) {
        result[i] = a[i] * b[i];
    }
}