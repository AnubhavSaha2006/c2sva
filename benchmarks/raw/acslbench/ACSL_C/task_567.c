/*
 * Task 567 Natural Language:
 * Write an ACSL-annotated C function that finds the maximum element in an integer array `a` of
 * size `n`. The function requires a non-empty array (`n > 0`) with valid memory for all elements.
 * It ensures the result is the largest value present in the array.
 */

/*@ predicate MaxElem{L}(int e, int *a, integer n) = 
  @   \forall integer i; 0 <= i < n ==> e >= a[i] &&
  @   \exists integer i; 0 <= i < n && e == a[i];
  @*/

/*@ requires n > 0 && \valid_range(a, 0, n - 1);
  @ assigns \nothing;
  @ ensures MaxElem(\result, a, n);
  @*/
int maxelem (int a[], int n) {
    int m = a[0];
    /*@ loop invariant 0 < k <= n;
      @ loop invariant MaxElem(m, a, k);
      @ loop assigns k,m;
      @ loop variant n - k;
      @*/
    for (int k = 1; k < n; k++)
        if (a[k] > m)
            m = a[k];
    return m;
}