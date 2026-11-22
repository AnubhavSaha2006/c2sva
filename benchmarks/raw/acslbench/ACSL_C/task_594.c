/*
 * Task 594 Natural Language:
 * Write an ACSL-annotated C function that swaps the integer values stored at two memory locations
 * pointed to by `p` and `q`. The function requires that both pointers are valid and that the
 * values they point to are between -1000 and 10000. The function ensures that after the swap, the
 * value at `p` equals the original value at `q` and the value at `q` equals the original value at
 * `p`.
 */

/*@
  @ requires \valid(p);
  @ requires \valid(q);
  @ requires -1000 <= *p <= 10000;
  @ requires -1000 <= *q <= 10000;
  @ assigns *p, *q;
  @ ensures *p == \old(*q);
  @ ensures *q == \old(*p);
*/
void swap(int *p, int *q) {
    int temp = *p;
    *p = *q;
    *q = temp;
}