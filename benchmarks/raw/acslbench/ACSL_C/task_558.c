/*
 * Task 558 Natural Language:
 * Write an ACSL-annotated C function that calculates the sum of integers from 0 to n-1. The
 * function requires that n is between 0 and 1000 (0 ≤ n ≤ 1000) and ensures the result equals the
 * mathematical formula n*(n-1)/2.
 */

/*@ requires 0<=n<=1000;
    ensures \result == n*(n-1)/2;
*/

int sum(int n) {
      int sum=0;
/*@ loop invariant 0<=i<=n;
    loop invariant sum == i*(i-1)/2;
    loop assigns sum, i;
    loop variant n-i;
*/
      for(int i=0;i<n;i++) {
           sum+=i;
      }
      return sum;
}