/*
 * Task 365 Natural Language:
 * Write an ACSL-annotated C function that finds the index of the minimum element in an integer
 * array `t` of size `n`. The function requires a valid array of size `n > 0` and ensures the
 * returned index points to an element that is less than or equal to every other element in the
 * array.
 */

/*@ requires n>0; 
    requires \valid(t+(0..n-1));
    ensures \forall integer i; 0<=i<n ==> t[\result]<=t[i];
    ensures \exists integer i; 0<=i<n && t[\result]<=t[i];
*/

int minarray(int t[],int n) {
      int index=0;
/*@ loop invariant 0<=i<=n;
    loop invariant 0<=index<n;
    loop invariant \forall integer j; 0<=j<i ==> t[index]<=t[j];
    loop variant n-1-i;
*/
      for(int i=0;i<n;i++) {
            if ( t[i] < t[index] ) index = i;
      }
      return index;
}