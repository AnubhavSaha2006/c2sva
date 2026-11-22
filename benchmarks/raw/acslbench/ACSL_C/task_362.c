/*
 * Task 362 Natural Language:
 * Write an ACSL-annotated C function that toggles a counter between 0 and 1 for N iterations. The
 * function takes an unsigned integer N and returns either 0 or 1, depending on whether N is even
 * or odd. The ensures clause guarantees the result is always 0 or 1.
 */


/*@
    ensures 0 <= \result <= 1;*/
// */
int loop(unsigned N) {
	int c = 0;
    unsigned i;
    /*@
        loop invariant 0 <= c <= 1;
        loop invariant i <= N;
        loop variant N - i;
    */
	for (i = 0; i < N; ++i) {
		c = 1 - c;
	}
	return c;
}
