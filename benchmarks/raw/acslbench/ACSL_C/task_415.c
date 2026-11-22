/*
 * Task 415 Natural Language:
 * Write an ACSL-annotated C function that swaps the integer values pointed to by two valid
 * pointers p and q. The function ensures that after the swap, the value at p equals the original
 * value at q, and the value at q equals the original value at p.
 */

# pragma SeparationPolicy(none)

/*@
   requires \valid(p);
   requires \valid(q);


   assigns *p;
   assigns *q;

   ensures *p == \old(*q);
   ensures *q == \old(*p);
*/
void swap(int* p, int* q)
{
  int const save = *p;
  *p = *q;
  *q = save;
}

int main () {
  int p[2] = { 0,1};
  int *q = (int *)((char*)q+1);
  swap(p,q);
}
