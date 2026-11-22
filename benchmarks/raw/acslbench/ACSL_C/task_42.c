/*
 * Task 42 Natural Language:
 * Write an ACSL-annotated C function that performs element-wise division between two integer
 * arrays (a and b) of length n, returning a new double array. The function requires valid arrays,
 * non-zero divisors in b, and ensures each result element equals a[i]/b[i]. Returns NULL if
 * allocation fails.
 */

#include <stdlib.h>
#include <assert.h>

/*@
  requires n >= 0;
  requires \valid(a+(0..n-1)) && \valid(b+(0..n-1));
  requires \forall integer i; 0 <= i < n ==> b[i] != 0;
  assigns \nothing;
  ensures \forall integer i; 0 <= i < n ==> \result[i] == (double)a[i]/b[i];
  ensures \result != NULL;
*/
double* ElementWiseDivision(const int a[], const int b[], size_t n) {
    double* result = (double*)malloc(n * sizeof(double));
    if (result == NULL) return NULL;

    /*@
      loop invariant 0 <= i <= n;
      loop invariant \forall integer k; 0 <= k < i ==> result[k] == (double)a[k]/b[k];
      loop assigns i, result[0..n-1];
      loop variant n - i;
    */
    for (size_t i = 0; i < n; ++i) {
        result[i] = (double)a[i] / b[i];
    }

    return result;
}