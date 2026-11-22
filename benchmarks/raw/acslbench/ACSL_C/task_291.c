/*
 * Task 291 Natural Language:
 * Write an ACSL-annotated C program that initializes a global integer variable to 0 and passes its
 * address to a global pointer. The function `loop` requires that the global integer equals 0, the
 * pointer is valid, and the pointer points to the global integer.
 */


int global_i;
int* global_i_ptr = &global_i;
int global_i = 0;

/*@ requires global_i == 0;
    requires \valid(global_i_ptr);
    requires global_i_ptr == &global_i; */
void loop(void) { }

int main(void) {
  loop();
  return 0;
}
