/*
 * Task 448 Natural Language:
 * Write an ACSL-annotated C function that sets all even-indexed elements of array `a` of size `n`
 * to zero. The function requires that `n` is positive and the array is valid. The function ensures
 * that for all even indices `k` (0 ≤ k < n), the element `a[k]` equals zero.
 */

/*@
requires n > 0;
requires \valid(a + (0..n-1));
ensures \forall integer k; (0<=k<n) && (k%2==0) ==> (a[k] == 0);
assigns a[0..n-1];
*/
void func(int *a, int n) {
    /*@
        loop invariant 0 <= i <= n;
        loop invariant \forall integer k;  (0 <= k < i) && (k%2==0) ==> a[k] == 0;
        loop invariant \valid(a + (0..n-1));
        loop assigns i, a[0..n-1];
        loop variant n-i;
    */
    for (int i = 0; i < n; i++) {
        if (i%2==0) 
            a[i] = 0;
    }
}