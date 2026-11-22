/*
 * Task 349 Natural Language:
 * Write an ACSL-annotated C function that takes two valid integer pointers p and q and returns the
 * maximum value between the integers they point to. The function ensures the result is greater
 * than or equal to both *p and *q, and equals one of them.
 */

/*@ requires \valid(p) && \valid(q);
    ensures \result >= *p && \result >= *q;
    ensures \result == *p || \result == *q;
    assigns \nothing;
*/
int max_ptr ( int *p, int *q ) {
  if ( *p >= *q )
    return *p ;
  return *q ;
}
