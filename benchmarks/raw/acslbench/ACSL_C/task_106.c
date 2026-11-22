/*
 * Task 106 Natural Language:
 * Write an ACSL-annotated C function that counts occurrences of value x in array a of size n,
 * while accumulating the sum of matches in *sum. Requires non-empty array (n > 0) and positive x.
 * Returns count (0 ≤ count ≤ n) and ensures *sum equals count*x.
 */

/*@
    requires n > 0 && x > 0;
    requires \valid_read(a + (0..n-1));
    ensures \result >= 0 && \result <= n;
*/
int func(int *a, int n, int x, int *sum) {
    int p = 0;
    int count = 0;
    *sum = 0;
    /*@
        loop invariant 0 <= p <= n;
        loop invariant count <= p && *sum == count*x;
        loop assigns p, count, *sum;
    */
    while (p < n) {
        if (a[p] == x) {
            count = count + 1;
            *sum = *sum + x;
        }
        p = p + 1;
    }
    Label_a:
    *sum += 0;
    //@ assert \at(*sum, Label_a) == count*x;
    return count;
}

