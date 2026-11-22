/*
 * Task 211 Natural Language:
 * Write an ACSL-annotated C function that returns the maximum of two integers a and b. The
 * function ensures the result is greater than or equal to both input values and equals one of
 * them.
 */

/*@ ensures \result >= a && \result >= b;
    ensures \result == a || \result == b ;
*/
int max ( int a, int b ) {
  if ( a >= b )
    return a ;
  return b ;
}
