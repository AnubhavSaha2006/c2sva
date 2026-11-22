/*
 * Task 315 Natural Language:
 * Write an ACSL-annotated C function that takes two valid integer pointers p and q and returns the
 * maximum value between them. The function ensures the result is greater than or equal to both *p
 * and *q, and equals one of them. However, the implementation incorrectly sets both *p and *q to
 * zero before returning 0.
 */

/*@ requires \valid(p) && \valid(q);
    ensures \result >= *p && \result >= *q;
    ensures \result == *p || \result == *q;
*/
int max_ptr ( int *p, int *q ) {
  *p = 0; 
  *q = 0;
  return 0 ;
}
