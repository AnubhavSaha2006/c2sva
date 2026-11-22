/*
 * Task 366 Natural Language:
 * Write an ACSL-annotated C function that takes four integers (a, b, c, d) and returns integer b.
 * The function requires that b is between a and c (inclusive), and that a is between c and d
 * (inclusive). The function ensures the returned value maintains a specific ordering relationship
 * where the result is between a and c, and c is between the result and d.
 */


/*@ predicate inInterval(integer a, integer b, integer c) = a >= b && b <= c;

    predicate fourInOrder(integer a, integer b, integer c, integer d) = inInterval(b,a,c) && inInterval(c,b,d);

    predicate controlledEquality(integer a, integer b, boolean c) = a == b || c;
*/

/*@ requires inInterval(a,b,c);
    requires inInterval(c,a,d);
    ensures fourInOrder(\result,a,c,d);
*/
int toto(int a, int b, int c, int d){
    return b;
}