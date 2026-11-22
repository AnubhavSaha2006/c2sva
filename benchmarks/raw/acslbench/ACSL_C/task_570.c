/*
 * Task 570 Natural Language:
 * Write an ACSL-annotated C function that finds the minimum value in a subarray of an integer
 * array `t` starting from index `k` to the end of the array. The function requires a valid array
 * `t` of size `n` and a starting index `k` where `0 <= k < n`. It ensures the result is less than
 * or equal to every element in the subarray from index `k` to `n-1`.
 */

/*@ requires \valid(t+(0..n-1)) && 0 <= k < n;
  @ ensures \forall integer i; k <= i < n ==> \result <= t[i];
  @ assigns \nothing;
*/

int getMinSubarray(int t[], int n, int k) { 
  int res = t[k];
  /*@ loop invariant k+1 <= i <= n;
      @ loop invariant \forall integer j; k <= j < i ==> res <= t[j];
      @ loop assigns i, res;
      @ loop variant n - i;
    */	
  for (int i = k+1; i < n; i++) 
    if (t[i] < res) 
      res = t[i];
  return res;
}