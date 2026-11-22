/*
 * Task 137 Natural Language:
 * Write an ACSL-annotated C function that copies all elements from source array `t` to destination
 * array `s`. The function requires that the array size `n` is positive and that both arrays `s`
 * and `t` are valid for indices 0 to n-1. It ensures that after execution, every element in `s`
 * from index 0 to n-1 is equal to the corresponding element in `t`.
 */

/*@ requires n>0 && \valid(s+(0..n-1)) && \valid(t+(0..n-1));
    assigns s[0..n-1];
    ensures \forall integer i; 0<=i<n ==> s[i]==t[i];
*/
void copy1(int s[], int t[], int n) {
  /*@ loop invariant 0<=i<=n;
      loop invariant \forall integer j; 0<=j<i ==> s[j]==t[j];
      loop variant n-i;
  */
  for(int i=0;i<n;i++){
    s[i]=t[i];
  }
}