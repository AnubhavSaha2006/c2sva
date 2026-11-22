/*
 * Task 195 Natural Language:
 * Write an ACSL-annotated C function that returns the maximum of two integers a and b. The
 * function ensures the result is equal to either a or b.
 */

/*@ 
	ensures \result==a || \result==b;
*/
int max(int a,int b)
{
	if(a==b) return a;
	return a>b?a:b;
}
