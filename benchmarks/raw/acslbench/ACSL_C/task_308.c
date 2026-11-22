/*
 * Task 308 Natural Language:
 * Write an ACSL-annotated C function that swaps the values of two integers pointed to by
 * parameters a and b. The function requires that both pointers are valid and ensures that after
 * execution, *a contains the original value of *b and *b contains the original value of *a.
 */

/*@ requires a_valid: \valid(a);
    requires b_valid: \valid(b);
    ensures a_value: *a == \at(*b, Pre);
    ensures b_value: *b == \at(*a, Pre);
*/
void swap(int* a, int* b) {
  int tmp = *a;
  *a = *b;
  *b = tmp;
}
