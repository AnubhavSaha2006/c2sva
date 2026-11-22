/*
 * Task 397 Natural Language:
 * Write an ACSL-annotated C function that performs a right shift operation on an unsigned integer
 * `a` by 1 bit, which is equivalent to dividing `a` by 2. The function ensures the result equals
 * a/2 and assigns nothing.
 */


/*@ assigns \nothing;
    ensures \result == a / 2;
 */
unsigned rshift(unsigned a)
{
	return a >> 1;
}
