/*
 * Task 44 Natural Language:
 * Write an ACSL-annotated C function that subtracts two integer arrays element-wise (a - b) and
 * stores the results in a third array (result). The function requires the arrays a, b, and result
 * to be distinct valid arrays of length n, and ensures each result[i] equals a[i] - b[i].
 */

#include <stdlib.h>
#include <assert.h>

/*@
  requires n >= 0;
  requires \valid(a + (0..n-1));
  requires \valid(b + (0..n-1));
  requires \valid(result + (0..n-1));
  requires a != b && a != result && b != result;
  assigns result[0..n-1];
  ensures \forall integer i; 0 <= i < n ==> result[i] == a[i] - b[i];
*/
void SubtractSequences(const int a[], const int b[], int result[], size_t n) {
    /*@
      loop invariant 0 <= i <= n;
      loop invariant \forall integer j; 0 <= j < i ==> result[j] == a[j] - b[j];
      loop assigns i, result[0..n-1];
      loop variant n - i;
    */
    for (size_t i = 0; i < n; ++i) {
        result[i] = a[i] - b[i];
    }
}