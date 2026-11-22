/*
 * Task 429 Natural Language:
 * Write an ACSL-annotated C function that adds two integers pointed to by p and q. The function
 * requires valid read access to both pointers and that the integer values are within the range
 * -10,000,000 to 10,000,000. The function returns the sum of the two integers without modifying
 * any memory locations.
 */

/*@
	requires \valid_read(p) && \valid_read(q);
	requires -10000000<*p<10000000 && -10000000<*q<10000000;
	ensures \result==*p+*q;
	assigns \nothing;
*/
int add(int*p,int*q){
	return*p + *q ;
}
/*@ assigns \nothing; */
int main(){
	int a =24,b =42,x ;
	x = add(&a, &b) ;
	x = add(&a, &a) ;
}