/*
 * Task 494 Natural Language:
 * Write an ACSL-annotated C function that takes a character array `p` of size `n` and always
 * returns 0. The function requires that the array `p` is valid for reading in the range from index
 * 0 to n-1, and it assigns no memory.
 */

/*@ requires \valid_read(p+(0..n-1));
  @ ensures \result == 0;
  @ assigns \nothing;
  @ */
int somefun(char *p, int n) {
  return 0;
}