/*
 * Task 144 Natural Language:
 * Write an ACSL-annotated C function that finds the index of the minimum value in an integer array
 * `t` of size `n`. The function requires a valid array of size `n > 0` and ensures the returned
 * index is valid, points to the smallest element, and that all elements before this index are
 * strictly greater than the minimum value.
 */

/*@
requires n > 0;
requires \valid(t+(0..(n-1)));

ensures 0 <= \result < n;
ensures \forall integer k; 0 <= k < n ==>  t[k] >= t[\result];
ensures \forall integer k; 0 <= k < \result ==>  t[k] > t[\result];
 */

int min(int * t, int n) {
  int minInd = 0, i =0;

    /*@
      loop assigns i, minInd;

      loop invariant 0 <= i < n;
      loop invariant 0 <= minInd < n;
      loop invariant 0 <= minInd <= i;

      loop invariant \forall integer k; 0 <= k <= i ==>  t[minInd] <= t[k];
      loop invariant \forall integer k; 0 <= k < minInd ==>  t[k] > t[minInd];

      loop variant n-1-i;
     */
    while(i<n-1)
      if (t[++i] < t[minInd])
	minInd = i;
    //@ assert i == n-1;
    return minInd;
}
