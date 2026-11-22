/*
 * Task 580 Natural Language:
 * Write an ACSL-annotated C function that modifies three memory locations: the integer pointed to
 * by p, the integer pointed to by the pointer pointed to by q, and the global variable g. The
 * function requires valid pointers p, q, and *q, and assigns values to *p, **q, and g.
 */

/* UNSAFE because `assigns *q;` and not `assigns **q;`. */

int g = 42;

/*@
  requires \valid(p) && \valid(q) && \valid(*q);
  assigns *p, **q, g;
*/
void foo(int* p, int** q) {
  *p  = 42;
  **q = 42;
  g   = 42;
}