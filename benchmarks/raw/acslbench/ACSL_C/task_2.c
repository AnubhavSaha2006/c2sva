/*
 * Task 2 Natural Language:
 * Write an ACSL-annotated C function that computes the absolute value of an integer parameter 'a'
 * and returns it as a long. The function handles positive, zero, and negative inputs with
 * corresponding postconditions, and guarantees non-negative results. Special cases for INT_MIN are
 * avoided by using long return type.
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
