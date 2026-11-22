/*
 * Task 178 Natural Language:
 * Write an ACSL-annotated C function that increments the integer value pointed to by pointer p by
 * 1 and returns the new value. The function requires that the original value pointed to by p is
 * between -100 and 100 (inclusive) and that p is a valid pointer. The function ensures the return
 * value equals the original value plus one.
 */

int m = 0;
int t[10];
int *q = &m;

/*@ requires *p <= 100 && *p >= -100 && \valid(p);
    ensures \result == \old(*p)+ 1 ;  */

int incr(int *p)
{
  q = p;
  *p = *p + 1;   
  return *p;
};
