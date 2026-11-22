/*
 * Task 472 Natural Language:
 * Write an ACSL-annotated C function that swaps the values of two integers. The function requires
 * valid and separated pointers a and b, and ensures that after execution, *a contains the original
 * value of *b and *b contains the original value of *a.
 */

/*@
  requires \valid(a) && \valid(b);
  requires \separated(a,b);
  assigns *a, *b;
  ensures *a == \old(*b) && *b == \old(*a);
*/
void swap(int *a, int *b){
  int tmp = *a ; *a = *b ; *b = tmp ;
}

int main(){
  int x = 2;
  int y = 4;
  swap(&x, &y);
  //@ assert x == 4 && y == 2 ;
}