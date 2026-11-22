/*
 * Task 238 Natural Language:
 * Write an ACSL-annotated C function `bar` that takes a pointer `second` and returns 0. The
 * function requires that `second` is a valid pointer and ensures the result is always 0. A
 * separate function `sub` calls `bar` with a character pointer `c`.
 */


/*@ behavior b:
  requires \valid(second);
  ensures \result == 0;*/
int bar(int *second);

void sub (char * c) {
  bar(c);
  
}
