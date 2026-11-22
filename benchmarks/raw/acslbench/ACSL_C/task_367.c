/*
 * Task 367 Natural Language:
 * Write an ACSL-annotated C function that computes the factorial of a non-negative integer n. The
 * function requires n ≥ 0 and ensures the result equals the mathematical factorial of n, defined
 * recursively as fact(0) = 1 and fact(n) = n * fact(n-1). The function uses a loop to iteratively
 * calculate the factorial, maintaining the invariant that the current value f equals the factorial
 * of (i-1).
 */

// Note axiomatization with integer type, rather than int. 


/* inductive isfact(integer n, integer r) { 
  @  case isfact0:  
  @    isfact(0,1); 
  @  case isfactn:  
  @    \forall integer n, integer f;
  @    isfact(n-1,f) ==> isfact(n,f*n);
  @ } 
  @*/ 


/* axiomatic factorial {
  @
  @ logic integer fact (integer n);
  @
  @ axiom fact1: 
  @ \forall integer n; isfact (n,fact(n));
  @
  @ axiom fact2: 
  @ \forall integer n, integer f; isfact (n,f) ==> f==fact(n);
  @ }
  @
  @*/ 



/*@ logic integer fact (integer n) = 
  @ (n == 0) ? 1 : n * fact(n-1);
  @ 
  @*/

/*@ requires n >= 0;
  @ ensures fact(n) == \result;
  @ assigns \nothing;
  @*/
int factf (int n)
{
  int f = 1; 
  int i = 1 ; 
  
  /*@ loop invariant 1 <= i <= n+1 && f == fact(i-1);
    @ loop assigns i, f;
    @ loop variant n-i;
    @*/
  while (i <= n) {
    f = f * i; 
    i = i + 1;
  }
  return f;
}

