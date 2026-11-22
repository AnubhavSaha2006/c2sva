/*
 * Task 258 Natural Language:
 * Write an ACSL-annotated C function that returns the minimum of two integers a and b. The
 * function ensures the result is less than or equal to both a and b, and that the result is equal
 * to one of the input parameters.
 */

/*@
  @ assigns \nothing;
  @ ensures \result <= a;
  @ ensures \result <= b;
  @ ensures \result == a || \result == b;
*/
int min(int a, int b) {
    return (a < b) ? a : b;
}