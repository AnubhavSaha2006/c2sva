/*
 * Task 525 Natural Language:
 * Write an ACSL-annotated C function that decrements a non-negative integer `c` to zero. The
 * function requires `c >= 0` and ensures the loop terminates when `c` becomes 0. The loop
 * invariant maintains that `c` is always non-negative and never exceeds its initial value.
 */

/*@
   requires c >= 0;
   assigns \nothing;
   ensures \true;
 */

void f (int c) {
	/*@ loop invariant c >= 0;
	    loop invariant c <= \at(c, Pre);
	    loop assigns c;
            loop variant c;
	 */
	while (c != 0) {
		c--;
	}
	return;
}