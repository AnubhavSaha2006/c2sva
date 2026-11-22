/*
 * Task 151 Natural Language:
 * Write an ACSL-annotated C function that returns the minimum of two integers a and b. The
 * function has two behaviors: when a is less than b, it returns a; when a is greater than or equal
 * to b, it returns b.
 */

/*@
	behavior a_less:
		assumes a<b;
		ensures \result == a;
	behavior b_less:
		assumes a>=b;
		ensures \result == b;
*/
int min(int a, int b){
	if(a<b)
		return a;
	else
		return b;
}