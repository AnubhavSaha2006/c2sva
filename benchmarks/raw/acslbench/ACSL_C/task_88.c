/*
 * Task 88 Natural Language:
 * Write an ACSL-annotated C function that returns 1 for any non-zero input parameter 'a'. The
 * function requires that 'a' is not zero and guarantees no memory modifications.
 */

/*@ 
    requires a!=0;
    ensures \result == 1;
    assigns \nothing;
*/
int func(int a) {
    int x, y;
    int sum, res;
    if (a == 0){
        x = 0; y = 0;
    }
    else {
        x = 5; y = 5;
    }
    sum = x + y; 
    res = 10/sum; 
    return res;
}