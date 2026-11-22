/*
 * Task 590 Natural Language:
 * Write an ACSL-annotated C function that takes two integers x and y as parameters, where x ranges
 * from -10 to 0 and y ranges from 0 to 5. The function performs arithmetic operations on x and y
 * and returns a result that is guaranteed to be between -5 and 10.
 */

/*@
requires -10 <= x <= 0 ;
requires 0 <= y <= 5 ;
ensures -5 <= \result <= 10 ;
assigns \nothing;
*/
int function(int x, int y) {
    int res ;
    y += 10 ;
    x -= 5 ;
    res = x + y ;
    //@ assert -15 <= res <= 15;
    return res ;
}