/*
 * Task 537 Natural Language:
 * Write three ACSL-annotated C functions that operate on integers within 32-bit range limits:

1.
 * Function f: Takes integers a and b. If a > 0, returns a + b; otherwise returns -1. Requires all
 * inputs and their sum to be within 32-bit integer range.

2. Function g: Takes integers a and b
 * and returns their sum. Requires all inputs and their sum to be within 32-bit integer range.

3.
 * Function h: Takes a boolean condition e and integers a, b. Returns a if e is true, otherwise
 * returns b. The function has complete and disjoint behaviors for both true and false conditions.
 */


/*@ requires a < 2147483647 && b < 2147483647 && a > -2147483648 && b > -2147483648 && a + b <= 2147483647 && a + b >= -2147483648;
  @ ensures a > 0 ==> \result == a + b;
  @ ensures a <= 0 ==> \result == -1;
  @ assigns \nothing;
*/
int f(int a, int b) {

	if (a > 0)
		return a + b;

	return -1;
}


/*@ requires a < 2147483647 && b < 2147483647 && a > -2147483648 && b > -2147483648 && a + b <= 2147483647 && a + b >= -2147483648;
  @ ensures \result == a + b;
  @ assigns \nothing;
*/
int g(int a, int b) {

	return a + b;

}

/*@ ensures \result == (e ? a : b) ;
    @ assigns \nothing;
    @ behavior POS:
    @   assumes e ;
    @   ensures \result == a;
    @ behavior NEG:
    @   assumes !e ;
    @   ensures \result == b;
    @ complete behaviors POS, NEG;
    @ disjoint behaviors POS, NEG;
*/
int h(int e,int a,int b) {

        if (e) return a; else return b;

}
