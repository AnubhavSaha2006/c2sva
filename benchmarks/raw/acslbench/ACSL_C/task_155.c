/*
 * Task 155 Natural Language:
 * Write an ACSL-annotated C function that swaps the values of two integers. The function takes two
 * integer pointers as parameters and ensures that after execution, the value at pointer a equals
 * the original value at pointer b, and the value at pointer b equals the original value at pointer
 * a. The function requires that both pointers are valid.
 */

/*@
 requires \valid(a) && \valid(b);
 ensures *a == \old(*b) && *b == \old(*a);
*/

void swap(int* a, int* b){

  int tmp = *a;
  *a = *b;
  *b = tmp;
 }

 int main()
 {
   int a=13,b=14;
   swap(&a,&b);
 }
