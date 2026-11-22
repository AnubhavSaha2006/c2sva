/*
 * Task 113 Natural Language:
 * Write an ACSL-annotated C function that computes the factorial of a non-negative integer n. The
 * function requires n ≥ 0 and ensures the result matches the mathematical factorial definition
 * (using an axiomatic specification). The implementation uses a while loop with appropriate
 * invariants tracking the computation progress.
 */

#include <stdio.h>

 

/*@
  axiomatic Factorial {
  logic integer fact(integer n);

 

  axiom case_n:
    \forall integer n;
    n >= 1 ==> fact(n) == n*fact(n-1);
  axiom case_0:
    fact(0) == 1;
  }
*/

 

/*@
  requires  n >= 0;
  ensures   \result == fact(n);
  assigns   \nothing ; 
*/

 

int factorial(int n) {

 

  int i = 1;
  int f = 1;

 

  /*@
    loop invariant f == fact(i-1);
    loop invariant 0 < i;
    loop invariant i <= n+1;
    loop assigns i, f;      // other variables are invariant
  */

 

  while (i <= n)  {
    f = f * i;
    i = i + 1;
  }
  return f;
}