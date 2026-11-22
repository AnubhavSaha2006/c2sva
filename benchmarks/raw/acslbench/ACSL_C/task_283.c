/*
 * Task 283 Natural Language:
 * Write an ACSL-annotated C function that performs integer division and modulus operations on two
 * unsigned integers x and y. The function computes the quotient and remainder of x divided by y,
 * storing the results in pointers q and r respectively. Requires valid pointers q and r, with y
 * between 1 and 999,999,999 and x between 0 and 999,999,999.
 */

/*@
	requires \valid(q) && \valid(r);
	requires 0<y<1000000000 && 0<=x<1000000000;
	assigns *q, *r ;
*/
void div_rem(unsigned x,unsigned y,unsigned* q,unsigned* r){
	*q = x / y ;
	//@ assert *q==x/y;
	*r = x % y ;
	//@ assert *r==x%y;
}
