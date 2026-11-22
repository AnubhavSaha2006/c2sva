/*
 * Task 251 Natural Language:
 * Write an ACSL-annotated C function that computes the absolute value of an integer x. The
 * function requires that x is greater than INT_MIN to prevent undefined behavior. It ensures that
 * for positive x, the result equals x, and for negative x, the result equals -x.
 */

// Version 2: Better specification

#include <limits.h>

/*@ requires x > INT_MIN;

    behavior positive:
    assumes x > 0;
    ensures \result == \old(x);
    
    behavior negative:
    assumes x < 0;
    ensures \result == \old(-x);
*/

int abs(int x) {
  if (x < 0)
	return -x;
  else
	return x;
}  

int main() {

abs(10);
abs(-20);
return 0;
}
