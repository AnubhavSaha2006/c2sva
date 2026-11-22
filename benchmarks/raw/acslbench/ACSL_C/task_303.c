/*
 * Task 303 Natural Language:
 * Write an ACSL-annotated C function that finds the index of the first occurrence of the minimum
 * value in an integer array `t` of size `n`. The function requires a non-negative array size `n`
 * and a valid array. If the array is empty (n=0), it returns 0. Otherwise, it returns an index `i`
 * (0 ≤ i < n) such that `t[i]` is the minimum value. Furthermore, for this index, all elements
 * before it are strictly greater than `t[i]`, ensuring it is the first occurrence of the minimum
 * value.
 */

/*@
requires n >= 0;
requires \valid(t+(0..(n-1)));

behavior empty :
assumes n==0;
ensures \result==0;

behavior not_empty:
assumes n>0;
ensures 0 <= \result < n;
ensures \forall integer k; 0 <= k < n ==>  t[k] >= t[\result];
ensures \forall integer k; 0 <= k < \result ==>  t[k] > t[\result];

complete behaviors empty, not_empty;
disjoint behaviors empty, not_empty;
 */

int min(int * t, int n) {
  if (n==0) {
    return 0;
  } else {
    int maxInd = 0;
    int i =0;

    /*@
      loop assigns i, maxInd;

      loop invariant 0 <= i <= n;
      loop invariant 0 <= maxInd < n;
      loop invariant 0 <= maxInd <= i;

      loop invariant \forall integer k; 0 <= k < i ==>  t[k] >= t[maxInd];
      loop invariant \forall integer k; 0 <= k < maxInd ==>  t[k] > t[maxInd];

      loop variant n-i;
     */
   
    for(i=0;i<n;i++) {
      if (t[i] < t[maxInd]) {
	maxInd = i;
      }
    }


    return maxInd;
  }
}
