/*
 * Task 534 Natural Language:
 * Write an ACSL-annotated C function that swaps the values of two integers pointed to by
 * parameters a and b. The function requires that both pointers are valid for reading and writing.
 * The function ensures that after execution, the value at a equals the original value at b, and
 * the value at b equals the original value at a.
 */

/*@
  requires \valid(a) && \valid(b) && \valid_read(a) && \valid_read(b);
  assigns  *a, *b;
  ensures  *a == \old(*b);
  ensures  *b == \old(*a);
*/
void foo(int* a, int* b) {
  int tmp = *a;
  *a = *b;
  *b = tmp;
}