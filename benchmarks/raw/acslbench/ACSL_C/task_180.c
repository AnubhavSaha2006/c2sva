/*
 * Task 180 Natural Language:
 * Write an ACSL-annotated C function that doubles an unsigned integer `a` using a left shift
 * operation. The function requires that `a * 2` does not exceed the maximum integer value
 * (INT_MAX) to prevent overflow. It ensures the result is exactly `a * 2` and assigns no memory.
 */

#define INT_MAX 2147483647

/*@ requires (a * 2) <= INT_MAX;
    assigns \nothing;
    ensures \result == a * 2;
 */
unsigned lshift(unsigned a)
{
	return a << 1;
}
