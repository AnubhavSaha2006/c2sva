/*
 * Task 154 Natural Language:
 * Write an ACSL-annotated C program containing three functions. The first function, `is_pos`,
 * checks if an integer is positive. The second function, `is_nat`, checks if an integer is non-
 * negative. The third function, `minus1`, takes a positive integer, subtracts one, and returns the
 * non-negative result.
 */

#include <limits.h>
/*@ predicate is_pos(int  x0) = (x0>0);*/
/*@
assigns \nothing;
ensures \result <==> is_pos(x0);
*/
int is_pos(int  x0) {
  int x2 = x0 > 0;
  return x2;
}
/*@ predicate is_nat(int  x3) = (x3>=0);*/
/*@
assigns \nothing;
ensures \result <==> is_nat(x3);
*/
int is_nat(int  x3) {
  int x5 = x3 >= 0;
  return x5;
}
/*@
requires is_pos(x6);
assigns \nothing;
ensures is_nat(\result);
*/
int minus1(int  x6) {
  int x8 = x6 - 1;
  return x8;
}
