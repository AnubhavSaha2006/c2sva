/*
 * Task 217 Natural Language:
 * Write an ACSL-annotated C function that returns the sign of an integer x. The function returns 1
 * if x is positive, 0 if x is zero, and -1 if x is negative. The ensures clause guarantees the
 * result is always one of these three values. The behaviors are complete and disjoint, meaning
 * they cover all possible inputs and do not overlap.
 */


/*@ assigns \nothing;
    ensures \result == 0 || \result == 1 || \result == -1;
    behavior positive:
       assumes x > 0;
       ensures \result == 1;
    behavior zero:
       assumes x == 0;
       ensures \result == 0;
    behavior negative:
       assumes x < 0;
       ensures \result == -1;
    complete behaviors;
    disjoint behaviors;
 */
int spec_sign(int x)
{
   return (x > 0) - (x < 0);
}

/* assigns \nothing;
    ensures \result == 0 || \result == 1 || \result == -1;
    behavior positive:
       assumes x > 0;
       ensures \result == 1;
    behavior zero:
       assumes x == 0;
       ensures \result == 0;
    behavior negative:
       assumes x < 0;
       ensures \result == -1;
    complete behaviors;
    disjoint behaviors;
 */
/*int sign2(int x)
{
    // assumes 32-bit int and 2s complement signed shifts work (implementation defined by C spec)
    return (x>>31) | ((unsigned)-x >> 31);
}*/


#ifdef OUT_OF_TASK
#include <stdio.h>

int main(void)
{
   printf("res: %d\n", spec_sign(-10));
   return 0;
}
#endif
