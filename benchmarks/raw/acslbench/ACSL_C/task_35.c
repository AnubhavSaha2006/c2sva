/*
 * Task 35 Natural Language:
 * Write an ACSL-annotated C function that returns the k-th element (1-based index) from integer
 * array 'arr' of length 'len'. The function requires that parameter 'k' is a valid position (1 ≤ k
 * ≤ len).
 */

#include <stdio.h>
#include <assert.h>

/*@
  requires k >= 1 && k <= len;
  assigns \nothing;
  ensures \result == arr[k-1];
*/
int KthElement(const int arr[], size_t len, size_t k) {
    int result = arr[k - 1];
    return result;
}