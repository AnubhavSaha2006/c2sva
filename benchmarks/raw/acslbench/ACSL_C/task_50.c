/*
 * Task 50 Natural Language:
 * Write an ACSL-annotated C function that computes the bitwise XOR between two integer arrays a
 * and b, storing the results in array result. The function requires equal array lengths len and
 * valid memory for all arrays. Each result[i] will contain a[i] XOR b[i].
 */

#include <stdlib.h>
#include <assert.h>

/*@
  requires len >= 0;
  requires \valid(a + (0..len-1));
  requires \valid(b + (0..len-1));
  requires \valid(result + (0..len-1));
  assigns result[0..len-1];
  ensures \forall integer i; 0 <= i < len ==> result[i] == (a[i] ^ b[i]);
*/
void BitwiseXOR(const int a[], const int b[], int result[], size_t len) {
    /*@
      loop invariant 0 <= i <= len;
      loop invariant \forall integer k; 0 <= k < i ==> result[k] == (a[k] ^ b[k]);
      loop assigns i, result[0..len-1];
      loop variant len - i;
    */
    for (size_t i = 0; i < len; ++i) {
        result[i] = a[i] ^ b[i];
    }
}