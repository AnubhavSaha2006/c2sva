/*
 * Task 255 Natural Language:
 * Write an ACSL-annotated C function that takes a non-zero integer parameter `a` and returns 1.
 * The function ensures no division by zero occurs by assigning values to local variables such that
 * their sum is always 10 when divided.
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