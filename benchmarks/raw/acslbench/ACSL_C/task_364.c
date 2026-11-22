/*
 * Task 364 Natural Language:
 * Write an ACSL-annotated C function that calculates the absolute distance between two integers.
 * The function takes two integer parameters a and b, and returns an unsigned integer representing
 * the absolute value of the difference between their absolute values. The specification requires
 * that both a and b are greater than SPEC_INT_MIN, but there is an error in this requirement. The
 * implementation converts the integers to their absolute values as unsigned numbers and returns
 * the difference. There is also an error in the commented-out implementation lines.
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
