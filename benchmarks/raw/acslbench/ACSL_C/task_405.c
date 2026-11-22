/*
 * Task 405 Natural Language:
 * Write an ACSL-annotated C function that takes two integers a and b as parameters, assigns the
 * value of a to b, and returns the value of b. The function requires that both a and b are within
 * the 32-bit signed integer range and ensures the return value equals the original value of a.
 */

/*@
	requires a >= -2147483648 && a <= 2147483647;
	requires b >= -2147483648 && b <= 2147483647;
	ensures \result == a;
*/
int trans(int a, int b){
	int c = a;
	b = c;
	return b;
}
