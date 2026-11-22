/*
 * Task 241 Natural Language:
 * Write an ACSL-annotated C function that takes a valid integer pointer `p` and returns the
 * integer value stored at the memory location pointed to by `p`. The function ensures the return
 * value equals the dereferenced value of `p`.
 */

/*@
  requires \valid(p);
  ensures \result == *p;
*/
int foo(int* p) {
  return *p;
}
