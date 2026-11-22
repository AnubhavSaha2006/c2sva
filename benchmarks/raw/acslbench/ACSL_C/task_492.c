/*
 * Task 492 Natural Language:
 * Write an ACSL-annotated C function that finds the index of the first occurrence of a value `val`
 * in an integer array `t` of size `n`. The function requires a valid array of size `n > 0`. It
 * returns the index if the value is found (ensuring the index is between 0 and n-1), or returns -1
 * if the value is not present in the array.
 */

/*@ requires \valid(t+(0..n-1)) && n>0;
    behavior success:
        assumes \exists integer i; 0<=i<n && t[i]==val;
        ensures 0<=\result<n;
    behavior failure:
        assumes \forall integer i; 0<=i<n ==> t[i]!=val;
        ensures \result==-1;
    complete behaviors success, failure;
    disjoint behaviors success, failure;
*/
int index(int t[], int val, int n){
  /*@ loop invariant 0<=i<=n;
      loop invariant \forall integer j; 0<=j<i ==> t[j]!=val;
      loop assigns i;
      loop variant n-i;
  */
  for(int i=0;i<n;i++){
    if(t[i]==val) return i;
  }
  return -1;
}