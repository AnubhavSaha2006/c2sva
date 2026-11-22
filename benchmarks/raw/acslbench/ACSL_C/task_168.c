/*
 * Task 168 Natural Language:
 * Write an ACSL-annotated C function that returns the maximum of two integers a and b. The
 * function ensures the result is greater than or equal to both inputs and equals one of them. The
 * global variable v must remain unchanged (v == 3) after calling the max function.
 */

/*@ ensures \result >= a && \result >= b ;
    ensures \result == a || \result == b ; */
int max(int a, int b);

extern int v ;
   
int main(){
  v = 3;
  int r = max(4,2);
  //@ assert v == 3 ;
}
