/*
 * Task 335 Natural Language:
 * Write an ACSL-annotated C function that finds an index of a maximum element in an integer array.
 * The function takes an array `a` and its length `len` as input, requires `len > 0` and a valid
 * array, and ensures the returned index is within bounds and points to an element that is greater
 * than or equal to all other elements in the array.
 */



/*@ requires len > 0;
  @ requires \valid(a+(0..len-1));
  @ ensures 0 <= \result < len;
  @ ensures \forall integer i; 0 <= i < len ==> a[i] <= a[\result];
  @ assigns \nothing; */
int exo1(int *a, int len) {
  int i = 0, j = len - 1;
  /*@ loop invariant 0 <= i <= j < len;
    @ loop invariant
    @  \forall integer k;
    @         (0 <= k < i || j < k < len)
    @     ==> (a[k] <= a[i] || a[k] <= a[j]);
    @ loop assigns i, j;
    @ loop variant j - i; */
  while (i < j)
    if (a[i] <= a[j]) i++;
    else j--;
  return i;
}
