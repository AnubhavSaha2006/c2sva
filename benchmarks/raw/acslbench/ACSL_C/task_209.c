/*
 * Task 209 Natural Language:
 * Write an ACSL-annotated C function that returns the maximum of two integers x and y. The
 * function ensures the result is greater than or equal to both input parameters.
 */

/*@  ensures  (\result >= x && \result >=y) ; */

int maxint (int x, int y) { 
  return (x > y) ? x : y; 
}

