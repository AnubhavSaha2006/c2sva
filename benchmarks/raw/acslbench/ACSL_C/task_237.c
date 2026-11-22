/*
 * Task 237 Natural Language:
 * Write an ACSL-annotated C function that conditionally assigns value n to either pointer p or q
 * based on n's value. If n > 0, assigns n to *p; otherwise assigns n to *q. Requires valid
 * pointers p and q.
 */

/*@

   requires \valid(p);

   requires \valid(q);

   behavior p_changed:

      assumes n >0;

      assigns *p;

      ensures *p == n;

    behavior q_changed:

      assumes n <= 0;

      assigns *q;

      ensures *q == n;

 */

void f(int n, int *p, int *q)

{

	if (n > 0) *p = n; else *q = n;

}





