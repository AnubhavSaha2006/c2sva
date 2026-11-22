/*
 * Task 541 Natural Language:
 * Write an ACSL-annotated C function that returns the last element of a character array. The
 * function requires a valid non-empty array and ensures the result equals the element at position
 * size-1. The parameters are a character pointer to the array and an integer size representing the
 * array length.
 */

/*@ requires
  @   size > 0 && \valid(t+(0..size-1));
  @ assigns \nothing;
  @ ensures \result == t[size-1];
*/
char in_ecount(char *t, int size) {
  return t[size -1];
}

void in_ecount_test() {
  char a[] = "test";
  char b = in_ecount(a, 5);
  //@ assert b == '\0';
  char c = in_ecount(a, 4);
    //@ assert c == 't';
}