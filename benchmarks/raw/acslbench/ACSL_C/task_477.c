/*
 * Task 477 Natural Language:
 * Write an ACSL-annotated C function that performs two operations: First, a function `sum` that
 * returns the bitwise OR of two characters `a` and `b`, ensuring the result's second bit is set if
 * and only if at least one input's second bit is set. Second, a function `rotate_left` that
 * rotates a 32-bit unsigned integer `x` left by one position, ensuring the most significant bit
 * moves to the least significant bit position and all other bits shift left accordingly.
 */



#define ST(a) (((a)&2)!=0)

/*@
  behavior sum_ok:
    ensures ok: ST(\result) <==> ST(a) || ST(b) ;

  complete behaviors sum_ok;
*/
char sum( char a , char b )
{ return a|b; }

#define BIT_TEST(x,n) (((x)&(1<<(n)))!=0)
/*@ requires \true;
  ensures bit_zero:
      BIT_TEST(\result,0) <==> BIT_TEST(x,31);
  ensures other_bits:
      \forall int k ; 0 <= k && k < 31
      ==> ( BIT_TEST(\result,1+k) <==>  BIT_TEST(x,k));
 */
unsigned rotate_left (unsigned x) {
  return (x << 1) | (x >> 31);
}
