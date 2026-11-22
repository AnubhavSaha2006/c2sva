/*
 * Task 387 Natural Language:
 * Write an ACSL-annotated C function that takes a pointer `p` to a finite float and a pointer `q`
 * to a const char. The function requires that the two pointers are comparable and that the value
 * pointed to by `p` is finite. The function returns nothing and has no other observable behavior.
 */

/*@ requires \pointer_comparable((void*)p,(void*)q) && \is_finite(*p) ; */
void f(float*p, char const * q) {
  return;
}
