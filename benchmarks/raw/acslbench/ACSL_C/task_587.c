/*
 * Task 587 Natural Language:
 * Write an ACSL-annotated C function that finds the index of the minimum element in an integer
 * array `t` of size `n`. The function requires a valid, non-empty array (`n > 0`). It ensures the
 * returned index is valid (0 ≤ index < n) and that the element at this index is less than or equal
 * to every other element in the array.
 */

/*@ requires \valid(t+(0..n-1)) && n>0;
    ensures 0<=\result<n;
    ensures \forall integer i; 0<=i<n ==> t[\result] <= t[i];
*/
int minIndex(int t[], int n){
  int index=0;

  /*@ loop invariant 0<=i<=n;
      loop invariant 0<=index<n;
      loop invariant \forall integer j; 0<=j<i ==> t[index] <= t[j];
      loop assigns index,i;
      loop variant n-i;
  */
  for(int i=0;i<n;i++){
    if(t[i]<t[index]) {
      index=i;
    }
  }
  return index;
}