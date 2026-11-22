/*
 * Task 363 Natural Language:
 * Write an ACSL-annotated C function that computes the greatest common divisor (GCD) of two
 * positive integers a and b using Euclid's algorithm. The function requires both inputs to be
 * positive (a > 0 and b > 0) and ensures the result equals the mathematical GCD of a and b. The
 * axiomatic definition specifies GCD properties including gcd(x,x)=x, commutativity
 * (gcd(x,y)=gcd(y,x)), and the additive property gcd(x+y,y)=gcd(x,y).
 */

/*@ axiomatic GCD {
  @ 	logic integer gcd(integer x, integer y);
  @		axiom gcd_equal:
  @			\forall integer x;
  @				 (x > 0) ==> gcd(x, x) == x;
		axiom gcd_comm:
			\forall integer x, integer y;
				(x > 0 && y > 0) ==> gcd(x, y) == gcd(y, x);
		axiom gcd_add:
			\forall integer x, integer y;
				(x > 0 && y > 0) ==> gcd(x + y, y) == gcd(x, y);
  @ }
 */ 

/*@ requires a > 0 && b > 0;
  @ assigns \nothing;
  @ ensures \result == gcd(a, b);
 */

int euclid(int a, int b) {
	int x = a;
	int y = b;

	/*@ loop invariant x > 0 && y > 0;
	  @ loop invariant gcd(x, y) == gcd(a,b);
	  @ loop assigns x, y;
	  @ loop variant \max(x, y);
	 */

	while (x != y) {
		if (x > y) {
			x -= y;	
		} else {
			y -= x;
		}
	}

	return x;
}
