/*
 * Task 546 Natural Language:
 * Write an ACSL-annotated C function that returns the absolute value of an integer x. The function
 * requires that x is greater than the minimum integer value (-2147483648) and ensures the result
 * is non-negative and equals either x or -x.
 */

/*@ requires x > -2147483648; 
  @ ensures \result >= 0;
  @ ensures \result == x || \result == -x; 
  @ assigns \nothing; 
  @*/
int abs(int x) {
  if (x >= 0) return x;
  return -x;
}