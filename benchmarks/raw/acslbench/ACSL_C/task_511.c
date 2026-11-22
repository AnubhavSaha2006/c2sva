/*
 * Task 511 Natural Language:
 * Write an ACSL-annotated C function that calculates the square of an integer n by summing the
 * first n odd numbers. The function requires that n is between 0 and 1000 (0 ≤ n ≤ 1000) and
 * ensures the result equals n².
 */

/*@ requires 0 <= n <= 1000;
    ensures \result == n*n;
    assigns \nothing;
*/

int oddsum(int n) {
      int sum=0;
/*@ loop invariant 0<=i<=n;
    loop invariant sum==i*i;
    loop assigns i, sum;
    loop variant n-i;
*/
    for(int i=0;i<n;i++) {
          sum += i*2 + 1;
    }
     return sum;
}