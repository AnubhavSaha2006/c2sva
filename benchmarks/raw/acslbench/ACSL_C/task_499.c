/*
 * Task 499 Natural Language:
 * Write an ACSL-annotated C function that computes the remainder of integer division `a / b`. The
 * function requires that the divisor `b` is not zero and that the operation does not cause a
 * specific integer overflow (when `a` is the minimum integer value and `b` is -1). The function
 * ensures the result equals the mathematical modulo operation `a % b`.
 */

/*@ requires b != 0 && (a != -2147483648 || b != -1); // Prevent potential overflow
	assigns \nothing;
	ensures \result == (a % b);
 */
int mod(int a, int b)
{
	return a % b;
}