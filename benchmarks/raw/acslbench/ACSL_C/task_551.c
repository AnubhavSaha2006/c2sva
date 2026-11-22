/*
 * Task 551 Natural Language:
 * Write an ACSL-annotated C function that searches for an element `x` in an integer array `a` of
 * size `n`. The function returns 1 if `x` is found in the array and 0 if it is not found. The
 * function requires that the array size `n` is greater than 0 and that the memory for the array is
 * valid for reading. The behaviors are complete and disjoint, covering both the presence and
 * absence of `x` in the array.
 */

/*@
requires n > 0;
requires \valid_read(a + (0..n-1));
assigns \nothing;

behavior present:
    assumes \exists integer k; 0 <= k < n && x == a[k];
    ensures \result == 1;

behavior not_present:
    assumes \forall integer k; 0 <= k < n ==> x != a[k];
    ensures \result == 0;

complete behaviors present, not_present;
disjoint behaviors present, not_present;
*/
int arraySearch(int *a, int x, int n) {
    int p = 0;
    /*@
        loop invariant  0 <= p <= n;
        loop invariant \forall integer k; 0 <= k < p ==> x != a[k];
        loop assigns p;
        loop variant n - p;
    */
    while (p < n) {
        if (a[p] == x) {
            return 1;
        }
        p++;
    }
    return 0;
}