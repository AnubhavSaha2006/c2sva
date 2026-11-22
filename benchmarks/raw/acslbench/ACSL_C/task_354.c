/*
 * Task 354 Natural Language:
 * Write an ACSL-annotated C function that adds two integers a and b. The function requires that
 * the sum a+b does not cause integer overflow (stays within the 32-bit signed integer range). The
 * function ensures the result equals the mathematical sum of a and b.
 */

/*@ requires -2147483648<a+b<2147483647;
ensures \result==a+b;

*/
int add(int a,int b){
	return a+b ;
}
