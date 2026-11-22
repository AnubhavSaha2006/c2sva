/*
 * Task 95 Natural Language:
 * Write an ACSL-annotated C function that computes the greatest common divisor (GCD) of two non-
 * negative integers a and b using Euclid's algorithm. The function handles edge cases where either
 * input is zero and recursively reduces the problem size until the GCD is found.
 */

/*@
  axiomatic LCM {
  logic integer lcm(integer m, integer n);
  
  }
*/
/*@
  requires a>=0 && b>=0;
  ensures (a==0)&&(\result==b) || (b==0)&&(\result==a) || (a==b)&&(\result==a);
  ensures (a!=0)&&(a>b)&&(\result==gcd(a-b, b) || (b>a)&&(\result==gcd(b-a,a)));
*/

//(a>b)&&(\result==gcd(a-b, b) || (b>a)&&(\result==gcd(b-a,a)))

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

int main()
{
    int a = 98, b = 56;
    return 0;
}