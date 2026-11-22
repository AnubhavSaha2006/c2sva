/*
 * Task 377 Natural Language:
 * Write an ACSL-annotated C program that defines two functions, `foo` and `bar`, and a `Pair`
 * structure. The program uses overloaded predicates named `Positive` to check if integer types and
 * the `Pair` structure contain positive values. The function `foo` takes a positive short integer
 * `a` and returns it, ensuring the result is positive. The function `bar` takes a positive integer
 * `a`, creates a `Pair` where both members are set to `a`, and returns it, ensuring the resulting
 * pair is also positive.
 */



/*@
   predicate Positive(int a) = 0 < a;
*/

/*@
   predicate Positive(short a) = 0 < a;
*/

/*@
   requires Positive(a);

   assigns \nothing;

   ensures  Positive(\result);
*/
int foo(short a)
{
    return a;
}


struct Pair
{
    int first, second;
};

typedef struct Pair Pair;

/*@
  predicate Positive(Pair p) = Positive(p.first) && Positive(p.second);
*/


/*@
   requires Positive(a);
   assigns \nothing;
   ensures Positive(\result);
*/
Pair bar(int a)
{
    Pair p;
    p.first  = a;
    p.second = a;

    return p;
}
