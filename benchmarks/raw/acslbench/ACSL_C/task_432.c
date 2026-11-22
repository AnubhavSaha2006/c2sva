/*
 * Task 432 Natural Language:
 * Write an ACSL-annotated C function that returns its input integer `x`. The function requires
 * that `x` is non-negative (x >= 0) and ensures the returned result equals `x`.
 */

/*@
  requires x >= 0;
  ensures \result == x;
  assigns \nothing;
*/
int foo(int x) {
  return x;
}