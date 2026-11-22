/*
 * Task 1 Natural Language:
 * Write an ACSL-annotated C function that computes the absolute value of an integer parameter `a`.
 * The function handles positive, zero, and negative inputs, ensuring the result is always non-
 * negative. Special cases include when `a` equals `SPEC_INT_MIN + 1` to avoid overflow.
 */

#define SPEC_INT_MIN -2147483648
#define SPEC_INT_MAX 2147483647

/*@ requires SPEC_INT_MIN < a <= SPEC_INT_MAX;
    requires SPEC_INT_MIN < a;
    assigns \nothing;
    ensures \result >= 0;
    behavior positive:
       assumes a > 0;
       ensures \result == a;
    behavior zero:
       assumes a == 0;
       ensures \result == 0;
       ensures \result == a;
    behavior negative:
       assumes a < 0;
       ensures \result == -a;
    complete behaviors;
    disjoint behaviors;
 */
int spec_abs2(int a)
{
   int abs;

   if (a < 0) {
      abs = -a;
   } else {
      abs = a;
   }

   return abs;
}

#ifdef OUT_OF_TASK
#include <stdio.h>
int main(void)
{
   printf("res: %d\n", spec_abs2(SPEC_INT_MIN + 1));
   return 0;
}
#endif
