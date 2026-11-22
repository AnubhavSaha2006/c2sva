/*
 * Task 574 Natural Language:
 * Write an ACSL-annotated C function that returns the maximum of two integers a and b. The
 * function ensures the result is greater than or equal to both a and b, and is equal to at least
 * one of them.
 */

/*@ ensures \result >= a && \result >=b && (\result == a || \result == b);
*/
int max(int a, int b) {
  return a > b ? a : b;
}