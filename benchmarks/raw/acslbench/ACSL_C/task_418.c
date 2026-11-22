/*
 * Task 418 Natural Language:
 * Write an ACSL-annotated C function that calculates the absolute difference between two unsigned
 * integers `a` and `b`. The function ensures the result is the maximum of `a` and `b` minus the
 * minimum of `a` and `b`.
 */

/*@ assigns \nothing;
    ensures \result == \max(a, b) - \min(a, b);
 */
unsigned distance(unsigned a, unsigned b)
{
	return (a > b) ? (a - b) : (b - a);
}
