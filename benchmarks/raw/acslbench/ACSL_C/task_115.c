/*
 * Task 115 Natural Language:
 * Write an ACSL-annotated C function that calculates the sum of digits of a positive integer num.
 * The function requires num to be greater than 0 and returns the accumulated sum of all digits.
 */

/*@
    requires num > 0;
    ensures sum = 
*/
int func(int num) {
    int i = 0;
    int sum = 0;
    /*@
        loop invariant sum == (sum + i);
        loop assigns num, i, sum;
    */
    while(num>0) {
        i = num%10;
        sum += i;
        num /= 10;
    }
    return sum;
}