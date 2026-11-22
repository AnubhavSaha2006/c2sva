/*
 * Task 240 Natural Language:
 * Write an ACSL-annotated C program that performs a cyclic permutation of three integer values.
 * The function `permut` takes three pointers `a`, `b`, and `c` as input, which must point to
 * valid, separate memory locations. The function swaps the values such that the original value of
 * `b` is assigned to `a`, the original value of `c` is assigned to `b`, and the original value of
 * `a` is assigned to `c`. A helper function `swap` is used to exchange the values of two integers.
 */

/*@ requires a_valid: \valid(a);
    requires b_valid: \valid(b);
    assigns *a, *b;
    ensures a_value: *a == \at(*b,Pre);
    ensures b_value: *b == \at(*a, Pre);
*/
void swap(int* a, int *b);

/*@ requires a_valid: \valid(a);
    requires b_valid: \valid(b);
    requires c_valid: \valid(c);
    requires sep: \separated(a,b,c);
    ensures a_value: *a == \at(*b, Pre);
    ensures b_value: *b == \at(*c, Pre);
    ensures c_value: *c == \at(*a, Pre);
*/
void permut(int* a, int *b, int *c) {
  swap(a,b);
  swap(b,c);
}
