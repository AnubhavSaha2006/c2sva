/*
 * Task 372 Natural Language:
 * Write an ACSL-annotated C function that takes an integer pointer as input and returns 0. The
 * function requires that the input pointer is valid and ensures the result is always 0.
 */

/* run.config
   DONTRUN: linked with first which is the real test.
*/
/*@ behavior b:
  requires \valid(third);
  ensures \result == 0;*/
int bar(int *third) {
  third=(int*)*third;
  return 0;
}
