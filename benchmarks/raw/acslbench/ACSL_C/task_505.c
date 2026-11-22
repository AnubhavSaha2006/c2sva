/*
 * Task 505 Natural Language:
 * Write an ACSL-annotated C function that checks if an integer array `a` of size `n` is a
 * palindrome. The function requires a valid array of size `n > 0` and returns 1 if the array is a
 * palindrome (reads the same forwards and backwards), and 0 otherwise.
 */

/*@
  @ requires n > 0;
  @ requires \valid(a+(0..n-1));
  @ 
  @ assigns \nothing;
  @ behavior true_:
  @   assumes \forall integer i; 0 <= i < n / 2 ==> a[i] == a[n - i - 1];
  @   ensures \result == 1;
  @ behavior false_:
  @   assumes \exists integer i; 0 <= i < n / 2 &&  a[i] != a[n - i - 1];
  @   ensures \result == 0;
  @ complete behaviors true_, false_;
  @ disjoint behaviors true_, false_;
*/
int palindrome(int a[], int n) {
    /*@
      @ loop invariant 0 <= i <= n / 2;
      @ loop invariant \forall integer j; 0 <= j < i ==> a[j] == a[n - j - 1];
      @ loop assigns i;
      @ loop   variant n - 2 * i;
    */
    for (int i = 0; i < n / 2; ++i) {
        if (a[i] != a[n - i - 1]) return 0;
    }
    return 1;
}