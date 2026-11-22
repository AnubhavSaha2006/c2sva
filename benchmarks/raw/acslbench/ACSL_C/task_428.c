/*
 * Task 428 Natural Language:
 * Write an ACSL-annotated C function that reads and returns the integer value pointed to by
 * pointer p. The function requires that p is a valid readable pointer and ensures the result
 * equals the value stored at *p.
 */

/*@
  requires \valid_read(p);
  ensures \result == *p;
*/
int foo(int* p) {
  return *p;
}