/*
 * Task 326 Natural Language:
 * Write an ACSL-annotated C function that searches for the first occurrence of a value `val` in an
 * integer array `a` of size `n`. The function returns the index of the first occurrence if found
 * (ensuring all previous elements are not `val`), or returns `n` if the value is not present in
 * the array. The function requires a valid array of size `n > 0`.
 */

/*@
  @ requires n > 0;
  @ requires \valid(a+(0..n - 1));
  @ 
  @ assigns \nothing;
  @ behavior okay_:
  @   assumes \exists int i; 0 <= i < n && a[i] == val;
  @   ensures 0 <= \result < n;
  @   ensures \forall int i; 0 <= i < \result ==> a[i] != val;
  @   ensures a[\result] == val;
  @ behavior not_okay_:
  @   assumes \forall int i; 0 <= i < n &&  a[i] != val;
  @   ensures \result == n;
*/
int find(int a[], int n, int val) {
    /*@
      @ loop invariant  0 <= i <= n;
      @ loop invariant \forall int j; 0 <= j < i ==> a[j] != val;
      @ loop   variant n - i;
    */
    for (int i = 0; i < n; ++i) {
        if (a[i] == val) return i;
    }
    return n;
}