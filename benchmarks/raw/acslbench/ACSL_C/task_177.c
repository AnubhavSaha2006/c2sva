/*
 * Task 177 Natural Language:
 * Write an ACSL-annotated C function that finds all indices in an integer array `v` of size `n`
 * where the element is even (as determined by a helper predicate function). The function stores
 * the found indices in an output array `o` in increasing order and returns the total count of such
 * indices. The function requires a valid, non-empty input array `v` and a valid output array `o`
 * of at least size `n`.
 */

/*@ ensures \result==0 || \result==1;
    assigns \nothing;
 */
int predicate(int v) {
  return v % 2 == 0;
}

/*@ requires n > 0;
    requires \valid(v+(0..n-1));
    requires \valid(o+(0..n-1));
    ensures 0 <= \result <= n;
    ensures \forall int i; 0 <= i < \result ==> 0 <= o[i] < n;
    ensures \forall int i; 0 < i < \result ==> o[i-1] < o[i];
    assigns o[0..n-1];
 */
int index_where(int* v, int n, int* o) {
  int r = 0;
  /*@
    loop invariant 0 <= i <= n;
    loop invariant 0 <= r <= i;
    loop invariant \forall int j; 0 <= j < r ==> 0 <= o[j] < i;
    loop invariant \forall int j; 0 < j < r ==> o[j-1] < o[j];
    loop assigns i, r, o[0..n-1];
    loop variant n-i;
   */
  for (int i = 0; i < n; i++) {
    if (predicate(v[i])) {
      o[r++] = i;
    }
  }
  return r;
}
