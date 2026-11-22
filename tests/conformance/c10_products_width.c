/* rule: nested multiplications need wider integers than 2x the operand width */
/*@ requires -1000 <= x <= 1000 && -1000 <= y <= 1000 && -1000 <= z <= 1000;
    assigns \nothing;
    ensures \result == x * y * z + x; */
long cube_ish(int x, int y, int z)
{
  return (long)x * y * z + x;
}
