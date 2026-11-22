/*
 * Task 557 Natural Language:
 * Write an ACSL-annotated C function that searches for a value `v` in an integer array `a` of size
 * `n`. The function returns the index of the first occurrence of `v` if found, otherwise it
 * returns -1. The function requires that the array size `n` is non-negative and that the array `a`
 * is valid for indices 0 to n-1.
 */

/*@ requires n >= 0 && \valid_range(a, 0, n - 1);
  @ assigns \nothing;
  @ behavior success:
  @   assumes \exists integer k; 0 <= k < n && a[k] == v;
  @   ensures 0 <= \result < n && a[\result] == v;
  @ behavior failure:
  @   assumes \forall integer k; 0 <= k < n ==> a[k] != v;
  @   ensures \result == -1;
  @ complete behaviors success, failure;
  @ disjoint behaviors success, failure;
  @*/
int search (int a[], int n, int v) {
    /*@ loop invariant 0 <= i <= n;
      @ loop invariant \forall integer k; 0 <= k < i ==> a[k] != v;
      @ loop assigns i;
      @ loop variant n - i;
      @*/
    for (int i = 0; i < n; i++)
        if (a[i] == v)
            return i;
    return -1;
}