/*
 * Task 12 Natural Language:
 * Write an ACSL-annotated C function that calculates the absolute distance between two integers (a
 * and b). The function computes the difference between their absolute values and returns the
 * result as an unsigned integer. The specification requires that both inputs (a and b) are greater
 * than INT_MIN to avoid undefined behavior.
 */

#define SPEC_INT_MIN -2147483648
#define SPEC_INT_MAX 2147483647


/*@ requires \true;
    requires a > SPEC_INT_MIN; // Найдите ошибку в спецификации. Ответ.
    requires b > SPEC_INT_MIN; // Найдите ошибку в спецификации. Ответ.
    assigns \nothing;
    ensures \result == \abs(\abs(a) - \abs(b));
 */
unsigned distance_abs(int a, int b)
{
   unsigned ua = a < 0 ? -a : a;
   unsigned ub = b < 0 ? -b : b;
   //unsigned ua = a < 0 ? ((unsigned)-((long)a)) : a; // Найдите ошибку в реализации. Ответ.
   //unsigned ub = b < 0 ? ((unsigned)-((long)b)) : b; // Найдите ошибку в реализации. Ответ.

   return ua > ub ? ua - ub : ub - ua;
}

#ifdef OUT_OF_TASK
#include <stdio.h>

int main(void)
{
   printf("res: %u\n", distance_abs(3,30));
   printf("res: %u\n", distance_abs(-5,-20));
   printf("res: %u\n", distance_abs(7,-10));
   printf("res: %u\n", distance_abs(-2,15));
   return 0;
}
#endif
