/*
 * Task 184 Natural Language:
 * Write an ACSL-annotated C function that swaps the values of two integers. The function takes two
 * integer pointers as parameters and ensures that after execution, the value at the first pointer
 * equals the original value at the second pointer, and vice versa.
 */

 /*@
 requires \valid(a) && \valid(b);
 assigns *a, *b;
 ensures *a == \old(*b);
 ensures *b == \old(*a);
 */
 void swap(int* a, int* b){
 int tmp = *a;
 *a = *b;
 *b = tmp;
 }

 int main(){
 int a = 42;
 int b = 37;

 swap(&a, &b);

 //@ assert a == 37 && b == 42;

 return 0;
 }
