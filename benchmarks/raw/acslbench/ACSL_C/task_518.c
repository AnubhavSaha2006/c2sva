/*
 * Task 518 Natural Language:
 * Write an ACSL-annotated C function that calculates the square of an integer n by summing the
 * first n odd numbers. The function requires that n is between 0 and 1000 (0 ≤ n ≤ 1000) and
 * ensures the result equals n².
 */

/*@
requires 0 <= n <= 1000;
ensures \result == n * n;
assigns \nothing;
*/
int sumOdd(int n){
  int res=0;
  int i=0;
  /*@
    loop invariant 0<=i<=n;
    loop invariant res == i*i;
    loop assigns res,i;
    loop variant n-i;
  */
  while(i<n){
    res+=i*2+1;
    i++;
  }
  return res;
}