/*
 * Task 101 Natural Language:
 * Write an ACSL-annotated C function that takes two integers x (between -10 and 0) and y (between
 * 0 and 5), performs arithmetic operations, and returns a result guaranteed to be between -10 and
 * 10. The implementation includes an intermediate assertion verifying res is between -15 and 5.
 */

/*@
    requires -10 <= x <= 0 ;
    requires 0 <= y <= 5 ;
    ensures -10 <= \result <= 10 ;
*/
int function(int x, int y) {
    int res ;
    y += 10 ;
    x -= 5 ;
    res = x + y ;
    //@ assert -15 <= res <= 5;
    return res ;
}