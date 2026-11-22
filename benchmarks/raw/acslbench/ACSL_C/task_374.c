/*
 * Task 374 Natural Language:
 * Write an ACSL-annotated C function that swaps the values of two integer pointers p and q. The
 * function ensures that after execution, the value pointed to by p equals the original value
 * pointed to by q, and the value pointed to by q equals the original value pointed to by p. The
 * function assigns (modifies) the values at the memory locations pointed to by p and q.
 */


/*@ ensures *q == \old(*p) && *p == \old(*q) ;
  assigns *p,*q;
 */
void f (int *p,int*q) {
  int t = *p;
  *p = *q;
  *q = t;
  return;
}

