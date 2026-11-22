/*
 * Task 389 Natural Language:
 * Write an ACSL-annotated C function that finds the minimum value in an integer array `t` of size
 * `n`. The function requires a valid, non-empty array (`n > 0`). It ensures the result is the
 * smallest element in the array, that this element exists in the array, and that no memory is
 * assigned during the operation.
 */

/*@ requires \valid(t+(0..n-1)) && n > 0;
  @ ensures  (\forall integer i; 0 <= i < n ==> \result <= t[i]) &&
  @   (\exists integer i; 0 <= i < n && \result == t[i]); 
  @ assigns \nothing;
*/

int getMin(int t[], int n) { 
  int res = t[0]; 
  /*@ loop invariant 1 <= i <= n &&
    @   (\forall integer j; 0 <= j < i ==> res <= t[j]) &&
    @   (\exists integer j; 0 <= j < i && res == t[j]); 
    @ loop assigns i, res;
    @ loop variant n - i;
    @*/
  for (int i = 1; i < n; i++) 
  {
    if (t[i] < res) 
      res = t[i];
  }
  return res;
}

