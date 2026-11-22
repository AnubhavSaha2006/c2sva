/*
 * Task 103 Natural Language:
 * Write an ACSL-annotated C function that performs binary search on a sorted array `a` of size `n`
 * to find element `x`. The function requires the array to be sorted in non-decreasing order and
 * returns the index of `x` if found, or -1 otherwise. The specification ensures complete behavior
 * coverage for both present and absent cases.
 */

/*@
    requires n > 0;
    requires \valid_read(a+(0..n-1));
    requires \forall integer k, l; 0 <= k <= l < n ==> a[k] <= a[l];

    ensures \result >= -1 && \result < n;

    behavior present:
        assumes \exists integer k ; 0 <= k < n && a[k] == x ;
        ensures a[\result] == x ;

    behavior not_present:
        assumes \exists integer k ; 0 <= k < n && a[k] != x ;
        ensures \result == -1;

    disjoint behaviors;
    complete behaviors;

*/
int binarysearch(int* a, int x, int n) {

    int low = -1;
    int high = n;
    int p;

    /*@
        loop invariant 0 <= low <= n  && 0 <= high <= n;
        loop invariant \forall integer k; (0 <= k < n) && (a[k] == x) ==> (low <= k < high);
        loop assigns low, high;
    */
    while (low+1 < high) {
        p = (low + high) / 2;
        if (a[p] == x) 
            return p;
        else 
            if (a[p] < x)
            low = p;
            else high = p;
    }
    return -1;
}