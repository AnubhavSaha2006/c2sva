/*
 * Task 368 Natural Language:
 * Write an ACSL-annotated C function that takes an integer `i` and returns it. The function
 * requires that the input `i` is greater than the size of an `int` type and ensures the returned
 * result is also greater than the size of the variable `i`.
 */

/*@ requires i > sizeof(int);
  @ ensures \result > sizeof(i);
  @*/
int f(int i) {
  return i;
}
