/*
 * Task 446 Natural Language:
 * Write an ACSL-annotated C function that unconditionally terminates the program with the given
 * exit state. The function takes an integer parameter `state` and ensures no memory is modified
 * before termination.
 */

/*@ assigns \nothing;
*/
void inconditional_exit(int state) {
  exit (state);
}