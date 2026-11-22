/*
 * Task 339 Natural Language:
 * Write an ACSL-annotated C function that returns the maximum value among three integers a, b, and
 * c. The function ensures the result equals the maximum of the three input values.
 */

/*@ assigns \nothing;
    ensures \result == \max(\max(a, b), \max(b, c));
 */
int max3(int a, int b, int c)
{
	return a > b ? (a > c ? a : c) : (b > c ? b : c);
}
