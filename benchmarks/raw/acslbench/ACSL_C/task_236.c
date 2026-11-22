/*
 * Task 236 Natural Language:
 * Write an ACSL-annotated C function that subtracts 1 from a positive integer x. The function
 * requires that x is positive (x > 0) and ensures the result is a natural number (result ≥ 0).
 */

//@ predicate is_pos(int x) = x > 0;
//@ predicate is_nat(int x) = x >= 0;

/*@ requires is_pos(x);
    ensures is_nat(\result);
*/
int minus1(int x) {
  return x-1;
}
