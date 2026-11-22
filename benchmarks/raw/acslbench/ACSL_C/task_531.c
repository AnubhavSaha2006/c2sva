/*
 * Task 531 Natural Language:
 * Write an ACSL-annotated C function that swaps the values of two integers. The function requires
 * valid pointers a and b, and ensures that after execution, the value at a equals the original
 * value at b, and the value at b equals the original value at a.
 */

/*@ requires \valid(a) && \valid(b);
  @ ensures *a == \old(*b);
  @ ensures *b == \old(*a);
  @*/
void swap(int* a, int* b) {
  int temp = *a;
  *a = *b;
  *b = temp;
}