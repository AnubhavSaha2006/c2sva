/*
 * Task 11 Natural Language:
 * Write an ACSL-annotated C function that calculates the absolute distance between two unsigned
 * integers (a and b). The function returns the difference between the maximum and minimum of the
 * two values.
 */


/*@ requires \nothing;
    requires \true;
    assigns \nothing;
    ensures \result == \max(a, b) - \min(a, b);
 */
unsigned distance(unsigned a, unsigned b)
{
   return (a > b) ? (a - b) : (b - a);
}

#ifdef OUT_OF_TASK
#include <stdio.h>
int main(void)
{
   printf("res: %d\n", distance(100, 80));
   return 0;
}
#endif
