/*
 * Task 124 Natural Language:
 * Write an ACSL-annotated C function that reverses an integer array in-place. The function takes
 * an array pointer `a` and its length `n`, requiring `n` to be positive and the array to be valid.
 * The postcondition ensures the array becomes symmetric (a[k] equals a[n-k-1] for all elements up
 * to the midpoint).
 */

/*@
    requires n > 0;
    requires \valid_read(a + (0..n-1));

    ensures \forall integer k; 0 <= k < n/2 ==> a[k] == a[n-k-1];
*/
void reverse(int *a, int n) {
    int i = 0;
    int j = n-1;
    /*@
        loop invariant i <= n/2;
        loop invariant j >= n/2;
    */
    while (i < n/2) {
        int temp = a[i];
        a[i] = a[j];
        a[j] = temp;
        i++;
        j--;
    }
}