/*
 * Task 287 Natural Language:
 * Write an ACSL-annotated C function that computes the absolute value of an integer `a`. The
 * function returns a non-negative long integer. For positive input, it returns the input
 * unchanged; for zero, it returns zero; for negative input, it returns the negation of the input.
 * The function handles all integer inputs and ensures the result is always non-negative.
 */

#define SPEC_INT_MIN -2147483648
#define SPEC_INT_MAX 2147483647

/*@ assigns \nothing;
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
long spec_abs1(int a)
{
   long abs;

   abs = a;
   if (a < 0) {
      abs = -abs;
   }

   return abs;
}

#ifdef OUT_OF_TASK
#include <stdio.h>

int main(void)
{
   printf("res: %ld\n", spec_abs1(SPEC_INT_MIN + 1));
   return 0;
}
#endif
