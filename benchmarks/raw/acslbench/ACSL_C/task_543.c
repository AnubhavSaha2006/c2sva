/*
 * Task 543 Natural Language:
 * Write an ACSL-annotated C program that defines three functions: an absolute value function
 * `abs(x)`, a maximum value function `max(x, y)`, and a maximum absolute value function
 * `max_abs(x, y)`. The program requires inputs to be greater than `INT_MIN` to prevent integer
 * overflow and ensures the functions return the correct absolute value, maximum value, and maximum
 * absolute value respectively without modifying any external state.
 */

/*@ logic integer abs(integer x) = x >= 0 ? x : -x;
*/

#include<limits.h>

/*@ requires x > INT_MIN;
    ensures (x >= 0 ==> \result == x) && 
      (x < 0 ==> \result == -x);
    assigns \nothing; */
int abs ( int x );

/*@ requires x > INT_MIN;
    requires y > INT_MIN;
    ensures \result >= x;
    ensures \result >= y;
    ensures \result == x || \result == y;
    assigns \nothing; */
int max ( int x, int y );

/*@ requires x > INT_MIN;
    requires y > INT_MIN;
    ensures \result >= abs(x) && \result >= abs(y);
    ensures \result == abs(x) || 
      \result == abs(y);
    assigns \nothing; */
int max_abs( int x, int y ) {
  return max(abs(x),abs(y));
}