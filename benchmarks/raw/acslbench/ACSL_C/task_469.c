/*
 * Task 469 Natural Language:
 * Write an ACSL-annotated C function that calculates the sum of the first n odd numbers. The
 * function requires that n is between 0 and 1000 (0 ≤ n ≤ 1000) and ensures the result equals n
 * squared. The function takes a single integer parameter n and returns an integer.
 */

/*@
  @ requires 0 <= n <= 1000;
  @ ensures \result == n * n;
  @ assigns \nothing;
*/
int sum_n_odds(int n) {
    int res = 0;
    /*@
      @ loop invariant 0 <= i <= n;
      @ loop invariant res == i * i;
      @ loop assigns res, i;
      @ loop   variant n - i;
    */
    for (int i = 0; i < n; ++i) {
        res += (2 * i) + 1;
    }
    return res;
}