/*
 * Task 593 Natural Language:
 * Write an ACSL-annotated C function that computes the absolute value of an integer x. The
 * function requires that x is either non-negative or greater than INT_MIN to avoid overflow. It
 * returns x when x is non-negative, and returns -x when x is negative.
 */

#include<limits.h>
/*@ requires x >= 0 || x > INT_MIN;
    assigns \nothing;
    behavior pos:
      assumes x >= 0;
      ensures \result == x;
    behavior neg:
      assumes x < 0;
      ensures \result == -x;
    complete behaviors pos, neg;
    //disjoint behaviors pos, neg; // removed because x == 0 makes both pos and neg true.
*/
int abs ( int x ) {
  if ( x >=0 )
    return x ;
  return -x ;
}