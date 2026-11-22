/*
 * Task 96 Natural Language:
 * Write an ACSL-annotated C function that returns the maximum of two integers x and y. The
 * function requires that both inputs are greater than INT_MIN and ensures the result is greater
 * than or equal to both inputs and exactly equals one of them.
 */

#include <limits.h>
/*@
requires INT_MIN<x && INT_MIN<y;
ensures \result>=x && \result>=y;
ensures \result==x || \result==y; 
*/
int max ( int x, int y ) {
    if ( x >=y ) 
        return x ;
    return y ;
}

 

void foo()
{
    int s = max(34,45);
    int t = max(-43,34);
    
}