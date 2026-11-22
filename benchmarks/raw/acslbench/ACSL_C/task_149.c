/*
 * Task 149 Natural Language:
 * Write an ACSL-annotated C program that models a state machine using a global variable `rr` with
 * an invariant (0 ≤ rr ≤ 50). The program includes functions to increment `rr` (opa), set a status
 * flag (opb), and assign a large value to `rr` (opc). The main function uses a switch statement on
 * `rr` to call these operations, with specific behaviors for different cases and a default case.
 */


int status=0;
int rr=1;
//@ global invariant inv : 0<=rr<=50;

/*@ requires rr<50;
  @ behavior j :
  @  ensures rr<51;
*/
void opa() {
  rr++;
}

void opb () {
  status=1;
}

void opc() {
  rr=9000;
}

int main(){

  switch (rr) {
  case 1 : 
    opa();
    break;
  case 3 : 
    opa();
  default : 
    opc();
  }
  opb();

  return 1;
}
