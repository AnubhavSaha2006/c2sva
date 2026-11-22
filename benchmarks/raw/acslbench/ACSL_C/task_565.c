/*
 * Task 565 Natural Language:
 * Write an ACSL-annotated C function that increments the integer value pointed to by p by 1. The
 * function requires that p is a valid pointer and the value *p is between -100 and 100 inclusive.
 * The function ensures that after execution, *p equals its original value plus 1, and returns the
 * new value of *p.
 */

/*@ requires \valid(p) && *p <= 100 && *p >= -100;
 assigns *p;
 ensures *p == \old(*p)+ 1 ;  */

int incr(int *p)
{
  *p = *p + 1;
  return *p;
};