/*
 * Task 256 Natural Language:
 * Write an ACSL-annotated C function that takes two integers a and b and returns b. The function
 * contains unreachable code paths including a division operation and an assertion that 0 equals 1,
 * which are never executed.
 */

/*@ requires \true;
    assigns \nothing;
    ensures \result == b;
 */
int deadcode(int a, int b)
{
	if (a > b) {
		if (a < b) {
			return a / b;
		} else if (a == b) {
			//@ assert 0 == 1;
		}
	}

	return b;
}
