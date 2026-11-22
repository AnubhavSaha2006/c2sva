/*
 * Task 487 Natural Language:
 * Write an ACSL-annotated C function that takes a pointer to an enum value (RED or BLUE) and
 * returns twice the enum's integer value. The function requires that the pointer is valid for
 * reading and points to the value BLUE, ensuring the result equals 2.
 */

#pragma CIVL ACSL
enum t{RED, BLUE};

/*@ requires \valid_read(a) && *a==BLUE;
  @ ensures \result == 2;
  @ assigns \nothing;
  @*/
int f(enum t* a){
  return (*a)*2;
}