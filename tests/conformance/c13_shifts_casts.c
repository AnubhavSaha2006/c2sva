/* rule: shifts, bitwise and, unsigned casts */
/*@ requires x <= 0xFFFF;
    assigns \nothing;
    ensures \result == ((x << 1) & 0xFF); */
unsigned low_byte_of_double(unsigned x)
{
  return (x << 1) & 0xFFu;
}
