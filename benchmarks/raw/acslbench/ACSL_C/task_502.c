/*
 * Task 502 Natural Language:
 * Write an ACSL-annotated C function that calculates the Greatest Common Divisor (GCD) of two
 * positive integers a and b using Euclid's algorithm. The function requires both inputs to be
 * greater than 0 (a > 0, b > 0) and ensures the result is the GCD, handling cases where one input
 * is zero or both are equal.
 */

/*@
  axiomatic LCM {
  logic integer lcm(integer m, integer n);
  
  }
*/
/*@ requires a>0 && b>0;
  decreases a+b;
  assigns \nothing;
  ensures a == 0 ==> \result == b;
  ensures b == 0 ==> \result == a;
  ensures a == b ==> \result == a;
*/
int gcd(int a, int b) {
    if (a == 0)
       return b;

    if (b == 0)
       return a;

    if (a == b)
        return a;

    if (a > b)
        return gcd(a-b, b);
    return gcd(a, b-a);
}

/*@ assigns \nothing; */
int main()
{
    int a = 98, b = 56;
    return 0;
}