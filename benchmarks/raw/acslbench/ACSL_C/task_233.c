/*
 * Task 233 Natural Language:
 * Write an ACSL-annotated C function that calculates the average of two integers a and b. The
 * function ensures the result equals (a + b) / 2 using a safe computation method that avoids
 * overflow. The function takes two integer parameters a and b and returns their average.
 */

/*@ assigns \nothing;
    ensures \result == (a + b) / 2;
 */
int avr(int a, int b)
{
	int avr;

	if (a >= 0 && b >= 0) {
		if (b > a) {
			avr = a + (b - a) / 2;
		} else {
			avr = b + (a - b) / 2;
		}
	} else if (a < 0 && b < 0) {
		if (b > a) {
			avr = b + (a - b) / 2;
		} else {
			avr = a + (b - a) / 2;
		}
	} else {
		avr = (a + b) / 2;
	}

	return avr;
}
