/*
 * Task 78 Natural Language:
 * Write an ACSL-annotated C function that finds the first occurrence of a target value in a sorted
 * array (arr). The function requires a non-empty array and returns the index of the first match,
 * or -1 if the target is not found. The array must be sorted in non-decreasing order.
 */

#include <stddef.h>

/*@
  requires \valid_read(arr + (0..len-1));
  requires len > 0;
  requires \forall integer i; 0 <= i < len-1 ==> arr[i] <= arr[i+1];
  assigns \nothing;
  ensures (0 <= \result < len ==> arr[\result] == target) &&
          (\forall integer i; 0 <= i < \result ==> arr[i] != target);
  ensures \result == -1 ==> \forall integer i; 0 <= i < len ==> arr[i] != target;
*/
int FindFirstOccurrence(const int arr[], size_t len, int target) {
    int index = -1;

    /*@
      loop invariant 0 <= i <= len;
      loop invariant index == -1 ==> \forall integer k; 0 <= k < i ==> arr[k] != target;
      loop invariant index != -1 ==> (0 <= index < i && arr[index] == target);
      loop invariant \forall integer k; 0 <= k < i ==> arr[k] <= target;
      loop assigns i, index;
      loop variant len - i;
    */
    for (size_t i = 0; i < len; ++i) {
        if (arr[i] == target) {
            index = (int)i;
            break;
        }
        if (arr[i] > target) {
            break;
        }
    }

    return index;
}