/*
 * Task 157 Natural Language:
 * Write an ACSL-annotated C function that computes the absolute value of an integer `v`. The
 * function requires that `v` is not equal to `INT_MIN` and ensures the result equals the absolute
 * value of `v`.
 */

#define INT_MIN (-2147483648)

/*@ requires v != INT_MIN;
    assigns \nothing;
    ensures \result == \abs(v);
 */
int abs(int v)
{
	return v >= 0 ? v : -v;
}
