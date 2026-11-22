/*
 * Task 216 Natural Language:
 * Write an ACSL-annotated C function that calculates the factorial of a non-negative integer `i`.
 * The function requires that the factorial value does not exceed the maximum unsigned long value
 * (SPEC_ULONG_MAX). It ensures the result equals the mathematical factorial of `i`, defined
 * axiomatically as factorial(0)=1 and factorial(i)=factorial(i-1)*i for i>0. The function uses a
 * decreasing loop that multiplies a running product by each integer from `i` down to 1.
 */

/*@ axiomatic Factorial {
    logic integer factorial(integer i);
    axiom nil:
       factorial(0) == 1;
    axiom step:
       \forall integer i; i >= 0 ==>
          factorial(i) == factorial(i - 1) * i;

    lemma non_negative:
       \forall integer i; i >= 0 ==>
          factorial(i) > 0;
    }
 */

#define SPEC_ULONG_MAX 18446744073709551615UL

/*@ requires factorial(i) <= SPEC_ULONG_MAX;
    assigns \nothing;
    ensures \result == factorial(i);
 */
unsigned long factorial(unsigned i)
{
   unsigned long f = 1;
   /*@ loop invariant 0 <= i;
       loop assigns f, i;
       loop variant i;
    */
   while (i) {
      f *= i--;
   }
   return f;
}

#ifdef OUT_OF_TASK
#include <stdio.h>

int main(void)
{
   printf("res: %lu\n", factorial(10));
   printf("res: %lu\n", factorial(20));
   return 0;
}
#endif
