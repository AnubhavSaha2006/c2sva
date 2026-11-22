/*
 * Task 390 Natural Language:
 * Write an ACSL-annotated C function that divides two integers a and b. The function requires that
 * the divisor b is not 0 and not -1, and ensures the result equals the integer division a / b.
 */

/*@ requires b != 0;
    requires b != -1;
    assigns \nothing;
    ensures \result == a / b;
 */
int divide(int a, int b)
{
	return a / b;
}
