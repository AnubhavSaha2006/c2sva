/*
 * Task 550 Natural Language:
 * Write an ACSL-annotated C function that adds two integers `a` and `b`. The function requires
 * that the sum `a + b` does not cause integer overflow (must be between `INT_MIN` and `INT_MAX`).
 * The function ensures the result equals the mathematical sum of `a` and `b`.
 */

 #include<limits.h>
/*@
    requires INT_MIN - b <= a <= INT_MAX - b;
    ensures \result == a+b;
    assigns \nothing;
*/
int add(int a,int b){
    return a+b;
}