/*
 * Task 353 Natural Language:
 * Write an ACSL-annotated C function that swaps the values of two integers pointed to by p and q.
 * The function requires that both pointers are valid and point to integers between -10000 and
 * 10000 (exclusive). The function ensures that after execution, the value at p equals the original
 * value at q, and the value at q equals the original value at p.
 */

/*@ requires \valid(p) && \valid(q);
    requires -10000<*p<10000 && -10000<*q<10000;
    assigns *p;
    assigns *q;
    ensures *p==\old(*q);
    ensures *q==\old(*p);
*/
void swap(int *p, int *q){
  int sum=*p + *q;
  *p=sum-*p;
  *q=sum-*p;
}