/*
 * Task 476 Natural Language:
 * Write an ACSL-annotated C function that demonstrates bit-shift operations on 32-bit and 64-bit
 * unsigned integers. The function takes a long integer parameter x and performs left shifts on
 * unsigned int and unsigned long long variables, with assertions verifying the results equal
 * specific hexadecimal values.
 */

/*@ requires 1==1;
*/
void test(long x)
{
    unsigned a = 0x10203040;
    unsigned b = (a << 16);
    
    unsigned long long al = a;
    unsigned long long bl = al << 16;
    
    //@ assert b == (unsigned)bl;
    //@ assert b == 0x30400000;
}