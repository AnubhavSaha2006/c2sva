/*
 * Task 206 Natural Language:
 * Write an ACSL-annotated C program containing two functions that search for a value `v` in an
 * array `p` of size `n`. Both functions require a valid, non-empty array (`n > 0`). The first
 * function, `member`, returns the first index where `v` is found, or `-1` if `v` is not in the
 * array. The second function, `member_noreturn`, also finds the first index of `v` but uses a
 * local variable to store the result instead of returning immediately, and returns `-1` if `v` is
 * not found.
 */

/*@ requires n>0 && \valid(p+(0..n-1));
    ensures \result==-1 ==> !(\exists int i; 0<=i<n && p[i]==v);
    ensures \result!=-1 ==> 0 <= \result < n && p[\result]==v;
    assigns \nothing;
*/
int member(int* p, int n, int v) {
  /*@
    loop invariant (0 <= i <= n);
    loop invariant !(\exists int j; 0 <= j < i && p[j]==v);
    loop assigns i;
    loop variant n-i;
  */
  for (int i=0; i<n; i++) {
    if (p[i]==v) return i;
  }
  return -1;
}

/*@ requires n>0 && \valid(p+(0..n-1));
    ensures \result==-1 ==> !(\exists int i; 0<=i<n && p[i]==v);
    ensures \result!=-1 ==> 0 <= \result < n && p[\result]==v;
    assigns \nothing;
*/
int member_noreturn(int* p, int n, int v) {
  int r = -1;
  /*@
    loop invariant (0 <= i <= n);
    loop invariant r==-1 ==> !(\exists int j; 0 <= j < i && p[j]==v);
    loop invariant r!=-1 ==> 0 <= r < n && p[r]==v;
    loop assigns i, r;
    loop variant n-i;
  */
  for (int i=0; i<n; i++) {
    if (r==-1 && p[i]==v) {
      r = i;
    }
  }
  return r;
}
