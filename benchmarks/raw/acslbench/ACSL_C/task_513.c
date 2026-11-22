/*
 * Task 513 Natural Language:
 * Write an ACSL-annotated C function that finds the maximum value in an integer array `a` of size
 * `n`. The function requires a valid, non-empty array (`n > 0`). It ensures the result is greater
 * than or equal to every element in the array and that the result is an element present in the
 * array.
 */

/*@ 
requires \valid_read(a + (0..n-1));
requires n > 0;

ensures \forall integer k;  0 <= k < n ==> \result >=  a[k];
ensures \exists integer k;  0 <= k < n && \result == a[k];

assigns \nothing;
*/
int arraymax(int* a, int n) {
  int i = 1;
  int max = a[0];

  /*@ 
     loop invariant \forall integer k; 0 <= k < n ==> (k < i ==> max >= a[k]);
     loop invariant \exists integer k;  0 <= k < i &&  max == a[k];
     loop invariant 0 <= i <= n;
     loop assigns i,max;
     loop variant n - i;
 */
  while (i < n) {
    // Beginning of loop
    if (max < a[i])
    max = a[i];
    i = i + 1;
    // End of loop: Loop invariant comes here
  }
  return max;
}
