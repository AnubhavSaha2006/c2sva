/*
 * Task 462 Natural Language:
 * Write an ACSL-annotated C function that finds the index of the minimum element in an integer
 * array `a` of a given positive `length`. The function requires a valid array and ensures the
 * returned index points to an element that is less than or equal to every other element in the
 * array.
 */

/*@ requires \valid(a+(0..length-1));
  @ requires length > 0;
  @ ensures \forall integer j; 0 <= j < length ==> a[\result] <= a[j];
  @ assigns \nothing;
*/
int find_min(int* a, int length) {
  int min, min_idx;
  min_idx = 0;
  min = a[0];

  /*@ loop invariant 1 <= i <= length;
      @ loop invariant 0 <= min_idx < length;
      @ loop invariant min == a[min_idx];
      @ loop invariant \forall integer j; 0 <= j < i ==> min <= a[j];
      @ loop assigns i, min_idx, min;
      @ loop variant length - i;
  */
  for (int i = 1; i < length; i++) {
    if (a[i] < min) {
      min_idx = i;
      min = a[i];
    }
  }
  /*@ assert \forall integer k; 0 <= k < length ==> a[min_idx] <= a[k]; */
  return min_idx;
}