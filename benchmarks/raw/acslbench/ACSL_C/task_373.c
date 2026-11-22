/*
 * Task 373 Natural Language:
 * Write an ACSL-annotated C function that checks if integer x is greater than 0 and integer y is
 * less than 10. The function returns 1 if both conditions are true, and 0 otherwise. The
 * specification uses predicates to define the conditions and includes two equivalent
 * implementations with different annotation styles.
 */

/*@ predicate pcond1(integer p) = (p>0)?\true:\false ;

 @ predicate pcond2(integer p) = (p<10)?\true:\false ;

 @*/



/*@

 //// ensures (pcond1(x) && pcond2(y)) ==> \result == 1 ;

 @ ensures \result == 1 <==> pcond1(x) && pcond2(y);

 @*/

int ftest1(int x, int y)

{

 return (x>0 && y<10);

}





/*@ predicate pcond3(integer p) = p > 0;

  @ predicate pcond4(integer p) = p < 10;

  @*/



/*@ behavior ok:

  @   assumes pcond3(x) && pcond4(y);

  @   ensures \result == 1;

  @ behavior ko:

  @   assumes !pcond3(x) || !pcond4(y);

  @   ensures \result == 0;

  @*/

int ftest2(int x, int y)

{

 return (x>0 && y<10);

}

