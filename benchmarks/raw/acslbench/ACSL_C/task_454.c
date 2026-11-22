/*
 * Task 454 Natural Language:
 * Write an ACSL-annotated C program with global variables `status` and `rr`, where `rr` must
 * satisfy the global invariant 0 ≤ rr ≤ 5000. The program contains two functions: `opa` which adds
 * two integers and assigns the result to `rr`, and `opb` which updates `status` and returns 3. The
 * main function conditionally calls `opa` to add 300 to `rr` if `rr < 5000`, then calls `opb` and
 * assigns its return value to `rr`.
 */

#include <limits.h>

int status=0;
int rr=1;
//@ global invariant inv : 0<=rr<=5000;

/*@ requires rr<5000 && i >= INT_MIN - j && i <= INT_MAX - j;
  @ assigns rr;
  @ ensures rr == i + j;
  @*/
void opa(int i, int j) {
  rr=i+j;
}


/*@ assigns status, \result;
  @ ensures \result == 3;
 */
int opb () {
  status=1;
  return status*3;
}

/*@ assigns rr, status;
 */
int main(){

  //@ assert rr < 5000;
  if (rr<5000) opa(rr,300);
  rr=opb();

  return 0;
}