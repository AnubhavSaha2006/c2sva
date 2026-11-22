/*
 * Task 123 Natural Language:
 * Write an ACSL-annotated C function that sets all even-indexed elements (0-based) of array `a`
 * with length `n` to zero while preserving odd-indexed elements. The function requires a valid
 * non-empty array and ensures all even positions contain zero.
 */

/*@
    requires n > 0;
    requires \valid_read(a + (0..n-1));
    ensures  \forall integer k; (0<=k<n) && (k%2==0) ==> (a[k] == 0);
*/
void func(int *a, int n) {
    /*@
        loop invariant 0 <= i <= n;
        loop invariant \forall integer k;  (0 <= k < i) && (k%2==0) ==> a[k] == 0;
        loop invariant \forall integer k;  (0 <= k < i) && (k%2==1) ==> a[k] == a[k];
        loop assigns i, a[0..(n-1)];
    */
    for (int i = 0; i < n; i++) {
        if (i%2==0) 
            a[i] = 0;
    }
}