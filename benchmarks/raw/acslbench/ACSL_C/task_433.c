/*
 * Task 433 Natural Language:
 * Write an ACSL-annotated C function that increments an integer n by 1. The function requires that
 * n is less than or equal to 2147483646 to prevent integer overflow and ensures the result equals
 * the original value of n plus 1.
 */

/*@ requires n <= 2147483646;
  ensures \result == \old(n) + 1 ;
 assigns \nothing; */

int incr(int n)
{
  return n+1;
};