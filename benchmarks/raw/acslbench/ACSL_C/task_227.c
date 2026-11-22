/*
 * Task 227 Natural Language:
 * Write an ACSL-annotated C function that returns the maximum of two integers a and b. The
 * function ensures the result is greater than or equal to both input values, equals one of them,
 * and corresponds to the logical max operation.
 */

/*@ logic integer max(integer a, integer b) =
          a > b ? a : b;
 */
/*@ assigns \nothing; 
    ensures \result >= a;
    ensures \result >= b;
    ensures \result == a || \result == b;
    ensures \result == max(a, b);
 */
int max(int a, int b)
{
   return a > b ? a : b;
}
