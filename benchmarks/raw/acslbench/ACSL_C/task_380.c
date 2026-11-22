/*
 * Task 380 Natural Language:
 * Write an ACSL-annotated C function that calculates the absolute difference between two unsigned
 * integers a and b. The function ensures the result equals the maximum of a and b minus the
 * minimum of a and b.
 */


/*@ //requires \nothing;
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
