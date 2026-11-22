/*
 * Task 215 Natural Language:
 * Write an ACSL-annotated C function that may terminate the program. The function takes a
 * condition `cond` and a value `val`. If the condition is false, it assigns `val` to a global
 * variable `status` and exits the program with status code 1. If the condition is true, the global
 * `status` remains unchanged.
 */

/*@ terminates \true;
  @ assigns \nothing;
  @ ensures \false;
  @ exits   \exit_status == status;
  @*/
void exit(int status);

int status;

/*@ terminates \true;
  @ assigns status;
  @ exits   !cond && \exit_status == 1 && status == val;
  @ ensures  cond && status == \old(status);
  @*/
void may_exit(int cond, int val) {
  if (! cond) {
    status = val;
    exit(1);
    }
}
