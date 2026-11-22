/*
 * Task 194 Natural Language:
 * Write an ACSL-annotated C function that returns the minimum of two integers, a and b. The
 * function ensures the result is less than or equal to both a and b, and is equal to at least one
 * of them.
 */

/*@ ensures \result<=a && \result<=b && (\result==a || \result==b);
*/
int min(int a, int b) {
  return (a<b)?a:b;
}