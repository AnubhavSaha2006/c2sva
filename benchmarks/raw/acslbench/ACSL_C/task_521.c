/*
 * Task 521 Natural Language:
 * Write an ACSL-annotated C function that returns the maximum of two integers, x and y. The
 * function ensures the result is equal to either x or y and is greater than or equal to both input
 * values.
 */

/*@
  ensures \result == x || \result == y;
  ensures \result >= x && \result >= y;
*/
int max(int x, int y) {
    if (x >= y) {
        return x;
    }
    return y;
}