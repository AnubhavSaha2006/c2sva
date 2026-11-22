/*
 * Task 356 Natural Language:
 * Write an ACSL-annotated C function that performs a binary search for a given key in a sorted,
 * non-empty array. The function requires the array to be sorted in non-decreasing order and
 * ensures it returns the index of the key if found, or the array length if the key is not present.
 * The parameters are a pointer to the array, the array length, and the search key.
 */

#include "stddef.h"


/*@
 requires pos: length > 0;
 requires array: \valid_read(a + (0 .. length - 1));
 requires sorted: \forall integer i, j; 0 <= i < j < length ==> a[i] <= a[j];
 assigns \nothing;
 behavior has_key:
   assumes has_key: \exists integer i; 0 <= i < length && a[i] == key;
   ensures 0 <= \result < length;
   ensures a[\result] == key;
behavior no_key:
   assumes no_key: \forall integer i; 0 <= i < length ==> a[i] != key;
   ensures \result == length;

complete behaviors;
disjoint behaviors;
*/
size_t binary_search(char* a, size_t length, char key) {
  size_t low = 0;
  size_t high = length - 1;
  /*@ loop invariant bounds: 0 <= low <= high + 1 <= length;
      loop invariant small_vals: \forall integer i; 0<=i<low ==> a[i] < key;
      loop invariant big_vals:
        \forall integer i; high < i < length ==> a[i] > key;
      loop assigns high, low;
      loop variant high - low;
  */
  while (high >= low) {
    // There was a quite well known bug in the original version of the code:
    // for big arrays, computing (low + high) / 2 might overflow when doing
    // the addition, resulting in an incorrect middle index.
    size_t middle = low + (high - low) / 2;
    if (a[middle] == key) return middle;
    if (a[middle] < key) low = middle + 1;
    else {
      // another subtle issue of the original code: if we're looking for a
      // small 'key', 'middle' will eventually become '0', in which case we know
      // that we won't find 'key'. And of course, we do not want to compute
      // 'middle - 1' there
      if (middle == 0) return length;
      else high = middle - 1;
    }
  }
  return length;
}
