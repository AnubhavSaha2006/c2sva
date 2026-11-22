/*
 * Task 294 Natural Language:
 * Write an ACSL-annotated C function that computes the absolute value of an integer `a`. The
 * function requires that `a` is greater than the minimum integer value. It ensures the result is
 * non-negative. For positive inputs and zero, the result equals the input; for negative inputs,
 * the result is the negation of the input. The function has no side effects.
 */

#define SPEC_INT_MIN -2147483648
#define SPEC_INT_MAX 2147483647

/*@ //requires SPEC_INT_MIN < a <= SPEC_INT_MAX;
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
