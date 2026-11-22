/*
 * Task 271 Natural Language:
 * Write an ACSL-annotated C function that returns the maximum of two integers a and b. The
 * function ensures the result is greater than or equal to both inputs and equals one of them.
 */

/*@ assigns \nothing; 
    ensures \result >= a;
    ensures \result >= b;
    ensures \result == a || \result == b;
 */
int max(int a, int b)
{
   return a > b ? a : b;
}
