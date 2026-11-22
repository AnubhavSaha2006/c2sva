/*
 * Task 489 Natural Language:
 * Write an ACSL-annotated C function that subtracts two integers a and b. The function requires
 * that the result of a - b does not cause integer overflow (INT_MIN ≤ a - b ≤ INT_MAX) and ensures
 * the result equals the mathematical subtraction a - b.
 */

#include <limits.h>
/*@
  requires INT_MIN <= a - b <= INT_MAX;
  ensures (\result == a - b);
  assigns \nothing;
*/
int sub(int a,int b)
{
  int diff =  a - b;
  return diff;
}
/*@ assigns \nothing; */
int main(void)
{
   int a = 7;
   int b = 6;
   sub(a,b);
   return 0;
}