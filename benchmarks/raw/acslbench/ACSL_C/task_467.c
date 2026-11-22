/*
 * Task 467 Natural Language:
 * Write an ACSL-annotated C function that returns the maximum of two integers x and y. The
 * function has two behaviors: when x ≥ y it returns x, and when x < y it returns y. The behaviors
 * are complete and disjoint, covering all possible cases.
 */

/*@ requires \true;
  @ behavior x_ge_y:
  @   assumes x >= y;
  @   ensures \result == x;
  @ behavior x_lt_y:
  @   assumes x < y;
  @   ensures \result == y;
  @ complete behaviors x_ge_y, x_lt_y;
  @ disjoint behaviors x_ge_y, x_lt_y;
  @*/
int max (int x, int y) {
  if (x >= y) {
    return x;
  } else {
    return y;
  }
}