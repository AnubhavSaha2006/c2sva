/*
 * Task 70 Natural Language:
 * Write an ACSL-annotated C function that performs element-wise modulo operation between two
 * integer arrays (a and b), returning a new array where each element is a[i] % b[i]. The function
 * requires non-zero divisors in array b and equal array lengths (len > 0).
 */

#include <stdlib.h>

/*@
  requires \valid(a + (0..len-1)) && \valid(b + (0..len-1));
  requires len > 0;
  requires \forall integer i; 0 <= i < len ==> b[i] != 0;
  assigns \nothing;
  ensures \valid(\result + (0..len-1));
  ensures \forall integer i; 0 <= i < len ==> \result[i] == a[i] % b[i];
*/
int* ElementWiseModulo(const int a[], const int b[], size_t len) {
    int* result = (int*)malloc(len * sizeof(int));
    
    /*@
      loop invariant 0 <= i <= len;
      loop invariant \forall integer j; 0 <= j < i ==> result[j] == a[j] % b[j];
      loop assigns i, result[0..len-1];
      loop variant len - i;
    */
    for (size_t i = 0; i < len; ++i) {
        result[i] = a[i] % b[i];
    }

    return result;
}