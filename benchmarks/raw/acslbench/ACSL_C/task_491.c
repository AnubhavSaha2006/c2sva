/*
 * Task 491 Natural Language:
 * Write an ACSL-annotated C function that computes the linear expression `a*x + b`. The function
 * requires that all intermediate calculations (`a*x` and `a*x + b`) remain within the range of
 * `INT_MIN` to `INT_MAX` to prevent integer overflow. The function ensures the result is within
 * the integer range and correctly equals `a*x + b`.
 */

#include "limits.h"

/*@
requires 
 a*x <= INT_MAX
 && a*x+b <= INT_MAX
 && a*x >= INT_MIN
 && a*x+b >= INT_MIN;
ensures \result >= INT_MIN && \result <= INT_MAX && \result == a*x+b;
*/
int f(int a,int b,int x) {
  return a*x+b;
}