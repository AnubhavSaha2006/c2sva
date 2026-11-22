/*
 * Task 470 Natural Language:
 * Write an ACSL-annotated C function that computes the absolute value of an integer x. The
 * function requires that x is greater than or equal to -2147483647, ensures the result is non-
 * negative, and returns -x if x is negative, otherwise returns x.
 */

/*@ requires x >= -2147483647;
	assigns \nothing;
	ensures \result >= 0;
	ensures x < 0 ==> \result == -x;
	ensures x >= 0 ==> \result == x;
*/
int abs(int x)
{
	if (x < 0)
		return -x;
	else
		return x;
}