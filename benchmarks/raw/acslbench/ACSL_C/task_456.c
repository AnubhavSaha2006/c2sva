/*
 * Task 456 Natural Language:
 * Write an ACSL-annotated C function that implements McCarthy's 91 function. The function takes an
 * integer n between -1000 and 1000 and returns 91 if n ≤ 100, otherwise returns n-10. The function
 * uses recursion with termination ensured by a variant clause.
 */

/*@
  requires -1000 <= n <= 1000; // Add a reasonable input range
  decreases n > 100 ? 1 : 102 - n;
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