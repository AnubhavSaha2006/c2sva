/*
 * Task 174 Natural Language:
 * Write an ACSL-annotated C function that calculates the absolute difference between two integers
 * a and b. The function requires both input integers to be within the range (-1,000,000,000,
 * 1,000,000,000) and ensures the result equals the absolute value of (b - a).
 */

/*@
	requires -1000000000<a<1000000000;
	requires -1000000000<b<1000000000;
	ensures b==\result+a && b>a || b==a-\result && a>=b;	
*/
int distance(int a,int b){
	if(a < b)
		return b - a ;
	else
		return a - b ;
}

