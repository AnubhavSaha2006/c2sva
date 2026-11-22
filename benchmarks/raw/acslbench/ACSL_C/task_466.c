/*
 * Task 466 Natural Language:
 * Write an ACSL-annotated C program with three functions: `a()`, `b()`, and `main()`. Function
 * `a()` returns 1, function `b()` returns 2, and `main()` calls `a()` and `b()` in a loop. The
 * loop in `main()` increments variable `x` by `b()`'s return value until `x` is at least 10, then
 * returns `a()`'s value. The program uses LTL properties for verification.
 */


/*@ requires \true;
  @ ensures 0<=\result<=1;
  @ assigns \nothing;
*/
int a() {
  return 1;
}

/*@ requires \true;
  @ ensures 1<=\result<=2;
  @ assigns \nothing;
*/
int b() {
  //call_to_an_undefined_function(); 
  return 2;
}

/*@ requires \true;
  @ ensures 0<=\result<=1;
  @ assigns \nothing;
*/
int main(){
  int x=a();
  /*@ loop invariant 0<=x<=12;
      loop assigns x;
      loop variant 10-x;
   */
  while (x<10) {
    x+=b();
  }
  return a();
}
