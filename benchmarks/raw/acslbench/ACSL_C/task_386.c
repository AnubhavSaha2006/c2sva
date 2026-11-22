/*
 * Task 386 Natural Language:
 * Write an ACSL-annotated C function that computes x raised to the power n using an efficient
 * exponentiation algorithm. The function requires n ≥ 0 and ensures the result equals the
 * mathematical definition of exponentiation. The implementation uses lemmas to verify the
 * correctness of the recursive power calculation logic and the loop invariants for the iterative
 * computation.
 */

#pragma JessieIntegerModel(math)

/*@ logic integer POW (integer x, integer n) =
  @   (n > 0) ? x * POW(x, n-1)
  @           : 1;
  @*/

/*@ requires  n >= 0;
  @ ensures   \result == POW(x, n);
  @*/
unsigned long pow (unsigned long x, unsigned int n)
{
  unsigned long r;

  /*@ loop invariant n >= 0;
    @ loop invariant r * POW(x, n) == \at(POW(x, n), Pre);
    @ loop variant n;
    @*/
  for (r = 1; n > 0; n /= 2, x = x*x)
    if (n % 2 == 1)
       r *= x;
  return r;
}

/*@ lemma pow_zero:
  @   \forall integer x;
  @     POW(x, 0) == 1;
  @*/

/*@ lemma pow_square_odd:
  @   \forall integer x, integer n;
  @     n >= 0 ==>
  @     n % 2 == 1 ==>
  @       POW(x, n) == x * POW(x*x, n/2);
  @*/

/*@ lemma pow_square_even:
  @   \forall integer x, integer n;
  @     n >= 0 ==>
  @     n % 2 == 0 ==>
  @       POW(x, n) == POW(x*x, n/2);
  @*/
