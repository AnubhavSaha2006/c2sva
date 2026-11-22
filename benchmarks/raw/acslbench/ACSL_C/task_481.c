/*
 * Task 481 Natural Language:
 * Write an ACSL-annotated C function that computes the absolute value of an integer n. The
 * function requires n to be within the valid integer range and ensures the result is non-negative.
 * For positive n, the result equals n; for negative n, the result equals -n.
 */

/*@
  requires -2147483647 <= n <= 2147483647;
  ensures \result >=0;
  assigns \nothing;

  behavior positive:
    assumes n >= 0;
    ensures \result == n;
    assigns \nothing;

  behavior negative:
    assumes n < 0;
    ensures \result == -n;
    assigns \nothing;

  complete behaviors positive, negative;
  disjoint behaviors positive, negative;
*/
int abs(int n){
    if(n<0){
        return -n;
    }
    return n;
}