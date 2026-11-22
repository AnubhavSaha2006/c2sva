/*
 * Task 509 Natural Language:
 * Write an ACSL-annotated C function that requires a valid read-only array `p` of size `n`. The
 * function ensures the return value is positive, assigns no memory, and takes a character pointer
 * `p` and an integer `n` as parameters.
 */

/*@ requires \valid_read(p+(0..n-1));
  @ ensures \result > 0;
  @ assigns \nothing;
  @ */
int somefun(char *p, int n) {
  return 1;
}