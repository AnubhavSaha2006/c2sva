/*
 * Task 453 Natural Language:
 * Write an ACSL-annotated C program containing an empty axiomatic block, a main function that
 * returns 0, and two functions f and g. Function f initializes a local integer variable to 0, and
 * function g takes a character pointer parameter. All functions are specified to assign nothing.
 */

/*@ axiomatic A {
}

*/

/*@ assigns \nothing; */
int main() { return 0; }

/*@ assigns \nothing; */
void f() {
  int x = 0;
  
}

/*@ assigns \nothing; */
void g(char*s);