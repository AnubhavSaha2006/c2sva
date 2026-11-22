/*
 * Task 336 Natural Language:
 * Write an ACSL-annotated C function that returns the maximum or minimum of two integers x and y
 * based on the input kind k (which must be either Max or Min). The function ensures the result is
 * one of the input values and satisfies the extremum condition: for Max, the result is greater
 * than or equal to both inputs; for Min, the result is less than or equal to both inputs.
 */

typedef enum { Max, Min } kind;
int extremum (kind k, int x, int y) {
  return ((k == Max ? x > y : x < y) ? x: y);
}
/*@ requires k == Max || k == Min;
    assigns \nothing;
    ensures \result == x || \result == y;
    behavior is_max:
      assumes k == Max;
      ensures \result >= x && \result >= y;
    behavior is_min:
      assumes k == Min;
      ensures \result <= x && \result <= y;
    complete behaviors is_max, is_min;
    disjoint behaviors is_max, is_min;
    complete behaviors;
    disjoint behaviors;
*/
int extremum (kind k, int x, int y);
