/*
 * Task 409 Natural Language:
 * Write an ACSL-annotated C function that calculates the sum of integers from 0 to n (inclusive).
 * The function requires that n is between 0 and 1000 (0 ≤ n ≤ 1000) and ensures the result
 * satisfies 2 * result == n * (n + 1).
 */

/*@
  @ requires 0 <= n <= 1000;
  @ ensures 2 * \result == n * (n + 1);
  @ assigns \nothing;
*/
int sum_n(int n) {
    int res = 0;

    /*@
      @ loop invariant 0 <= i <= n + 1;
      @ loop invariant 2 * res == i * (i - 1);
      @ loop   variant n - i + 1;
    */
    for (int i = 0; i <= n; ++i) {
        res += i;
    }
    return res;
}