/*
 * Task 165 Natural Language:
 * Write an ACSL-annotated C function that ensures the first pointer parameter points to the
 * smaller value between two integers. The function takes two valid integer pointers p and q. If *p
 * is already less than *q, both values remain unchanged. If *p is greater than or equal to *q, the
 * values are swapped so that *p becomes the smaller value and *q becomes the larger value.
 */



/*@ requires \valid(p) && \valid(q); 
    ensures *p <= *q; 
    behavior p_minimum: 
      assumes *p < *q; 
      ensures *p == \old(*p) && *q == \old(*q); 
    behavior q_minimum: 
      assumes *p >= *q; 
      ensures *p == \old(*q) && *q == \old(*p); 
    complete behaviors p_minimum, q_minimum; 
    disjoint behaviors p_minimum, q_minimum; 
*/
void max_ptr(int*p, int*q) { 
  if (*p > *q) { 
    int tmp = *p; 
    *p = *q; 
    *q = tmp; 
  } 
} 