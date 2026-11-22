/*
 * Task 182 Natural Language:
 * Write an ACSL-annotated C function that fills an integer array `t` of size `n` with a specified
 * value `val`. The function requires that the array size `n` is positive and the array is valid.
 * It ensures that after execution, every element in the array equals `val`.
 */

/*@ requires n>0 && \valid(t+(0..n-1));
    ensures \forall integer i; 0<=i<n ==> t[i] == val;
*/

void fill(int t[],int val,int n) {
/*@ loop invariant 0<=i<=n;
    loop invariant \forall integer j; 0<=j<i ==> t[j] == val;
    loop variant n-i;
*/
      for(int i=0;i<n;i++) {
            t[i]=val;
      }
}