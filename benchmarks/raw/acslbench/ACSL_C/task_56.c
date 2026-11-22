/*
 * Task 56 Natural Language:
 * Write an ACSL-annotated C function that extracts the first element from each sub-array in a 2D
 * array (lst) and returns them as a new array. The function requires valid input arrays and
 * ensures the result contains each lst[i][0] value. Parameter 'n' specifies the number of sub-
 * arrays.
 */

#include <stdlib.h>
#include <assert.h>

/*@
  requires \valid(lst + (0..n-1));
  requires \forall integer i; 0 <= i < n ==> \valid(lst[i] + (0..0));
  assigns \nothing;
  ensures \result != NULL;
  ensures \valid(\result + (0..n-1));
  ensures \forall integer i; 0 <= i < n ==> \result[i] == lst[i][0];
*/
int* GetFirstElements(int** lst, size_t n) {
    int* result = (int*)malloc(n * sizeof(int));
    if (result == NULL) return NULL;

    /*@
      loop invariant 0 <= i <= n;
      loop invariant \forall integer j; 0 <= j < i ==> result[j] == lst[j][0];
      loop assigns i, result[0..n-1];
      loop variant n - i;
    */
    for (size_t i = 0; i < n; ++i) {
        result[i] = lst[i][0];
    }

    return result;
}