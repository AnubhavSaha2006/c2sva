/*
 * Task 306 Natural Language:
 * Write an ACSL-annotated C function that returns the value pointed to by integer pointer p. The
 * function ensures the return value equals the value stored at the memory location pointed to by
 * p.
 */

/*@
  ensures \result == *p;
*/
int foo(int* p) {
  return *p;
}
