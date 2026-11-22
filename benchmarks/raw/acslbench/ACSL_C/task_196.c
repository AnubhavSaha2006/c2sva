/*
 * Task 196 Natural Language:
 * Write an ACSL-annotated C function that performs division and multiplication operations on
 * integers a and b. The function requires that b is not equal to 0 or -1, and ensures the result
 * equals the original value of a.
 */

/*@ requires b != 0;
    requires b != -1;
    assigns \nothing;
    ensures \result == a;
 */
int divmult(int a, int b)
{
	return (a / b) * b + (a % b);
}
