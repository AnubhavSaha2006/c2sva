/*
 * Task 532 Natural Language:
 * Write an ACSL-annotated C function that takes a pointer to a struct S containing character and
 * integer pointers, and returns the sum of the values pointed to by these pointers. The function
 * requires valid pointers and that the sum does not cause integer overflow, ensuring the result
 * equals the exact sum of the dereferenced values.
 */

struct S {
  char *x;
  int *y;
};

/*@
   requires \valid(s) && \valid_read(s->x) && \valid_read(s->y) &&
            (int)(*s->x) + *s->y >= -2147483648 && (int)(*s->x) + *s->y <= 2147483647;
   ensures \result == (int)(*s->x) + *s->y;
   assigns \nothing;
*/
int f(struct S* s) {
  return *s->x + *s->y;
}