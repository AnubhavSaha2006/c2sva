/*
 * Task 260 Natural Language:
 * Write an ACSL-annotated C function that swaps the values of two integers pointed to by
 * parameters a and b. The function requires valid pointers a and b, assigns values to *a and *b,
 * and ensures the values are exchanged such that *a equals the original *b and *b equals the
 * original *a.
 */

/*@
  requires \valid(a) && \valid(b);
  assigns  *a, *b;
  ensures  (*a) == \old(*b);
  ensures  (*b) == \old(*a);
*/
void foo(int* a, int* b) {
  int tmp = *a;
  *a = *b;
  *b = tmp;
}
