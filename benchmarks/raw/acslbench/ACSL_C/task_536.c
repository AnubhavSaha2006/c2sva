/*
 * Task 536 Natural Language:
 * Write an ACSL-annotated C function that increments an integer x by 1. The function requires that
 * x is non-negative and will not cause an integer overflow (x <= __INT_MAX__ - 1). The function
 * ensures the result is equal to x + 1 and does not modify any global state.
 */

/*@
  requires x >= 0 && x <= __INT_MAX__ - 1;
  ensures \result == x + 1;
  assigns \nothing;
*/
int foo(int x) {
  return x+1;
}