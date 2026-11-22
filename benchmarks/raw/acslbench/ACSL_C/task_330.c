/*
 * Task 330 Natural Language:
 * Write an ACSL-annotated C program that models a state machine using goto statements. The program
 * uses global variables `status` and `rr`, with a global invariant ensuring `rr` is between 0 and
 * 5000. The function `opa` increments `rr` and requires `rr` to be less than 5000, ensuring it
 * remains below 5001. The main function uses labeled blocks and goto commands to navigate between
 * operations, conditionally calling `opa`, `opb`, and `opc` based on the value of `rr`.
 */


int status=0;
int rr=1;
//@ global invariant inv : 0<=rr<=5000;

/*@ requires rr<5000;
  @ behavior j :
  @  ensures rr<5001;
*/
void opa() {
  rr++;
}

void opb () {
  status=1;
}

void opc () {
  rr=60000;
}

int main(){

  if (rr<5000) goto L;
  opc();

 L4:
  goto L5;

 L:
  opa();
  goto L2;
  opc();

 L6:
  return 1;

 L3:
  goto L4;

  opc();
  goto L2;

 L2 :
  goto L3;


 L5:
  opb();
  goto L6;
}
