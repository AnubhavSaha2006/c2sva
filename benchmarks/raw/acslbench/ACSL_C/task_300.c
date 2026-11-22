/*
 * Task 300 Natural Language:
 * Write an ACSL-annotated C function that takes a valid integer pointer `p` and always returns the
 * constant value 42. The function requires that the pointer `p` is valid, assigns no memory
 * locations, and ensures the result is exactly 42.
 */

/*@
  requires \valid(p);
  assigns \nothing;
  ensures \result == 42;
*/
int foo(int* p) {
  return 42;
}
