/*
 * Task 434 Natural Language:
 * Write an ACSL-annotated C program that defines several logical predicates for checking if a
 * pointer is null and uses them in assertions within function `f`. The program also includes
 * functions `g` and `h` with postconditions ensuring their return values satisfy specific logical
 * conditions.
 */

//@ predicate null(char *x) = x == 0;
//@ predicate eq(char *x, char *y) = x == y;
//@predicate my_null(char *x) = x == (void*)0;

void f(char *x) {
  x = 0;
  //@ assert x == (void*)0;
  //@ assert my_null(x);
  //@ assert null(x);
  //@ assert eq(x,0);
}

//@ ensures \result == \true;
int g() { return 1; }

/*@ predicate foo (integer x) = x != 0; */

/*@ ensures foo(1); */
int h() { return 1; }