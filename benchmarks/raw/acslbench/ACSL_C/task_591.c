/*
 * Task 591 Natural Language:
 * Write an ACSL-annotated C function that implements McCarthy's 91 function. The function takes an
 * integer n between -100 and 1000 and returns 91 if n ≤ 100, otherwise returns n-10. The function
 * uses recursion and includes termination guarantees.
 */

/*@
  requires -100 <= n <= 1000; // Added a reasonable range for n
  decreases n > 100 ? 0 : 101 - n; // Added decreases clause
  assigns \nothing;
  ensures ((n <= 100) ==> (\result == 91));
  ensures ((n > 100) ==> (\result == (n-10)));
*/
int foo(int n) {
  if (n > 100) {
    return n - 10;
  } else {
    return foo(foo(n + 11));
  }
}