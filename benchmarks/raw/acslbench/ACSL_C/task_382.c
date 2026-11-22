/*
 * Task 382 Natural Language:
 * Write an ACSL-annotated C function that sets the integer value at a given pointer p to zero. The
 * function requires that p is a valid pointer and ensures that after execution, the value pointed
 * to by p is 0.
 */

/*@
  requires \valid(p);
  ensures *p == 0;
*/
void main(int*p) {
  *p = 0;
  CEA_DUMP();
}
