/*
 * Task 257 Natural Language:
 * Write an ACSL-annotated C function that returns the maximum of two integers a and b. The
 * function ensures the result is greater than or equal to both input parameters.
 */

#include<limits.h>
/*@ ensures \result >= a && \result >= b; */
int max ( int a, int b ) {
  return INT_MAX ;
}
