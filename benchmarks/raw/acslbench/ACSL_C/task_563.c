/*
 * Task 563 Natural Language:
 * Write an ACSL-annotated C function that requires a valid pointer `p` to an array of at least 5
 * integers. The function assigns the value 1 to the first element of the array and ensures the
 * pointer remains valid for the same memory range upon return.
 */

/*@ requires \valid(p + (0..4));
  @ ensures \valid(p + (0..4));
  @*/
void validPointers(int * p) {
  *p = 1;
  return;
}