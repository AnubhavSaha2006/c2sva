/*
 * Task 383 Natural Language:
 * Write an ACSL-annotated C function that computes the modular inverse of n modulo d. The function
 * requires d > 0 and ensures the result is in the range [0, d-1). The parameters are unsigned
 * integers n and d.
 */

/*@
    requires d > 0;

    assigns \nothing;

    ensures 0 <= \result < d;
*/
static inline
unsigned int inverse_modulo(unsigned int n, unsigned int d)
{
  return d - 1 - (n % d);
}

