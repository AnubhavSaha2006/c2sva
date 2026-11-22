/*
 * Task 420 Natural Language:
 * Write an ACSL-annotated C function that calculates the integer square root of a non-negative
 * integer N. The function requires that N is between 0 and 1,000,000,000 (0 ≤ N ≤ 1000000000). It
 * ensures the result R satisfies R² ≤ N < (R+1)². The function iteratively increments R until
 * (R+1)² exceeds N, then returns R.
 */

/*@ 
  requires 0 <= N <= 1000000000;
  assigns \nothing;
  ensures \result * \result <= N ;
  ensures N < (\result+1) * (\result+1);
*/
int root(int N){
  int R = 0;
  /*@ 
    loop invariant 0 <= R * R <= N;
    loop assigns R;
    loop variant N-R;
  */
  while(((R+1)*(R+1)) <= N) {
    R = R + 1;
  }
  return R;
}