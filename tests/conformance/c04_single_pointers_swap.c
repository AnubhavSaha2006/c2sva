/* rule: single-object pointers (ap_ovld ports), \old of memory */
/*@ requires \valid(p) && \valid(q);
    requires \separated(p, q);
    assigns *p, *q;
    ensures *p == \old(*q);
    ensures *q == \old(*p); */
void swap(int *p, int *q)
{
  int t = *p;
  *p = *q;
  *q = t;
}
