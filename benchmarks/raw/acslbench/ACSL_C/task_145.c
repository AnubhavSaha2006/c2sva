/*
 * Task 145 Natural Language:
 * Write an ACSL-annotated C program with three functions. The `square` function computes and
 * returns the square of an integer input. The `pow4` function computes the fourth power of an
 * integer input by calling the `square` function twice. The `pow4b` function also computes the
 * fourth power of an integer input by squaring the input and then squaring the result. All
 * functions have no side effects.
 */

#include <limits.h>
/*@
assigns \nothing;
ensures (\result==(x0*x0));
*/
int square(int  x0) {
  int x2 = x0 * x0;
  return x2;
}
/*@
assigns \nothing;
ensures (\result==(((x6*x6)*x6)*x6));
*/
int pow4(int  x6) {
  int x8 = square(x6);
  int x9 = square(x8);
  return x9;
}
/*@
assigns \nothing;
ensures (\result==(((x15*x15)*x15)*x15));
*/
int pow4b(int  x15) {
  int x17 = square(x15);
  int x18 = x17 * x17;
  return x18;
}
