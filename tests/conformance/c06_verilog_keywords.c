/* rule: C names that are Verilog keywords (Vitis renames ports: buf -> buf_r) */
/*@ requires -1000 <= buf <= 1000 && -1000 <= input <= 1000;
    requires \valid_read(wire + (0 .. 1));
    requires -1000 <= wire[1] <= 1000;
    assigns \nothing;
    ensures \result == buf + input + wire[1]; */
int kw(int buf, int input, const int *wire)
{
  return buf + input + wire[1];
}
