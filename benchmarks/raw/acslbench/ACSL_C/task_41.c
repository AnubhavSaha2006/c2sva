/*
 * Task 41 Natural Language:
 * Write an ACSL-annotated C function that replaces the last element of array 'first' with all
 * elements from array 'second', storing the result in 'result'. Requires valid input arrays and
 * ensures proper concatenation of first[0..first_len-2] with second[0..second_len-1].
 */

#include <stdlib.h>
#include <assert.h>

/*@
  requires first_len > 0;
  requires \valid(first + (0..first_len-1));
  requires \valid(second + (0..second_len-1));
  requires \valid(result + (0..first_len+second_len-2));
  assigns result[0..first_len+second_len-2];
  ensures \forall integer i; 0 <= i < first_len-1 ==> result[i] == first[i];
  ensures \forall integer i; 0 <= i < second_len ==> result[first_len-1+i] == second[i];
*/
void ReplaceLastElement(const int* first, size_t first_len,
                       const int* second, size_t second_len,
                       int* result) {
    // Copy first array except last element
    /*@
      loop invariant 0 <= i <= first_len-1;
      loop invariant \forall integer j; 0 <= j < i ==> result[j] == first[j];
      loop assigns i, result[0..first_len-2];
      loop variant first_len-1 - i;
    */
    for (size_t i = 0; i < first_len-1; ++i) {
        result[i] = first[i];
    }

    // Append second array
    /*@
      loop invariant 0 <= i <= second_len;
      loop invariant \forall integer j; 0 <= j < i ==> result[first_len-1+j] == second[j];
      loop assigns i, result[first_len-1..first_len+second_len-2];
      loop variant second_len - i;
    */
    for (size_t i = 0; i < second_len; ++i) {
        result[first_len-1 + i] = second[i];
    }
}