/*
 * Task 334 Natural Language:
 * Write an ACSL-annotated C function that performs integer division of a by b, returning the
 * quotient and storing the remainder in *r. The function requires a >= 0, b > 0, and a valid
 * pointer r. It ensures the equation a = b * quotient + remainder holds.
 */

#include <stdio.h>

/*@ requires a >= 0 && b > 0;
  @ requires \valid(r);
  @ assigns *r;
  @ ensures a == b * \result + *r; 
 */

int idiv(int a, int b, int *r) {
	int q = 0;
	int p = a;
	/*@ assert a == b * q + p;*/
	
	/*@ loop invariant a == b * q + p;
	@ loop assigns q, p;
	@ loop variant p;
	*/
	while (p >= b) {
		q++;
		p -= b;
	}
	*r = p;

	return q;
}
/*
int main(int argc, char** argv) {
	int r = 0;
	int q = idiv(5, 2, &r);
	
	printf("q=%d, r=%d\n", q,r );
	return 0;
}*/
