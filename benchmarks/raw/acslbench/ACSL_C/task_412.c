/*
 * Task 412 Natural Language:
 * Write an ACSL-annotated C function that orders two integers pointed to by `a` and `b` so that
 * the value at `a` is less than or equal to the value at `b`. If the values are already in order,
 * it does nothing; otherwise, it swaps them using a helper `swap` function. The function requires
 * valid pointers `a` and `b`.
 */

/*@ requires \valid(a);
    requires \valid(b);

    assigns *a;
    assigns *b;

    ensures *a == \old(*b);
    ensures *b == \old(*a);
 */
void swap(int *a, int *b);

/*@ requires \valid(a);
    requires \valid(b);

    assigns *a, *b;

    ensures *a <= *b;
    behavior ret:
      assumes *a <= *b;
      ensures *a == \old(*a);
      ensures *b == \old(*b);
    behavior swap:
      assumes *a > *b;
      ensures *a == \old(*b);
      ensures *b == \old(*a);
    complete behaviors;
    disjoint behaviors;
 */
void max_ptr(int *a, int *b)
{
   if (*a <= *b) {
      return;
   } else {
      swap(a, b);
   }
}
