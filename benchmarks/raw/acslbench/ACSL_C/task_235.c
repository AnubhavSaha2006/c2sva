/*
 * Task 235 Natural Language:
 * Write an ACSL-annotated C function that performs a binary search for value `v` in a sorted array
 * `t` of size `n`. The function returns the index of `v` if found (a value between 0 and n-1), or
 * -1 if not found. The array must be valid and sorted in ascending order for the failure case
 * specification to hold.
 */

/*@ predicate sorted{L}(long *t, integer a, integer b) =
  @    \forall integer i,j; a <= i <= j <= b ==> t[i] <= t[j];
  @*/

/*@ requires n >= 0 && \valid_range(t,0,n-1);
  @ ensures -1 <= \result < n;
  @ behavior success:
  @   ensures \result >= 0 ==> t[\result] == v;
  @ behavior failure:
  @   assumes sorted(t,0,n-1);
  @   ensures \result == -1 ==>
  @     \forall integer k; 0 <= k < n ==> t[k] != v;
  @*/
int binary_search(long t[], int n, long v) {
  int l = 0, u = n-1;
  /*@ loop invariant
    @   0 <= l && u <= n-1;
    @ for failure:
    @   loop invariant
    @   \forall integer k; 0 <= k < n && t[k] == v ==> l <= k <= u;
    @ loop variant u-l;
    @*/
  while (l <= u ) {
    int m = (l + u) / 2;
    //@ assert l <= m <= u;
    if (t[m] < v) l = m + 1;
    else if (t[m] > v) u = m - 1;
    else return m;
  }
  return -1;
}

