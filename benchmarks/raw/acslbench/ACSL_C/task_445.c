/*
 * Task 445 Natural Language:
 * Write an ACSL-annotated C function `f` that takes two integer parameters `k` and `g`. The
 * function requires that `k` is greater than 0 (`k > 0`) and ensures the return value is less than
 * 1 (`\result < 1`). The function always returns 0.
 */

/*@ requires k>0;
  @ ensures \result<1;
  @*/
int f(int k, int g);


int f(int k, int g){
  return 0;
}

void main(){
  f(1,0);
}