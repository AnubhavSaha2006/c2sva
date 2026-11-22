/*
 * Task 585 Natural Language:
 * Write an ACSL-annotated C function that computes the absolute value of an integer x. The
 * function requires that x is not equal to the minimum integer value (INT_MIN) and ensures the
 * result is the non-negative absolute value of x.
 */

#include <limits.h>

/*@ requires x != INT_MIN;
  assigns \nothing;
  ensures x >= 0 ==> \result == x;
  ensures x < 0 ==> \result == -x;
*/

int abs(int x) {
  if (x < 0)
	return -x;
  else
	return x;
}

/*@
 assigns \nothing;
*/
int main() {
abs(10);
abs(-20);
return 0;
}