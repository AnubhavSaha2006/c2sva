/*
 * Task 292 Natural Language:
 * Write an ACSL-annotated C function that swaps the first two elements of an integer array. The
 * function requires a valid pointer to an array with at least two elements and ensures that after
 * execution, the first element equals the original second element and the second element equals
 * the original first element.
 */

/*@ requires \valid(p+ (0..1));
    ensures p[0] == \old(p[1]);
    ensures p[1] == \old(p[0]);
    assigns p[0], p[1];
*/
void array_swap(int* p) {
  int tmp = p[0];
  p[0] = p[1];
  p[1] = tmp;
}
