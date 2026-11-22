/*
 * Task 413 Natural Language:
 * Write an ACSL-annotated C program that defines a custom type `assert` as an integer and a global
 * variable `behavior`. The main function increments `behavior` if it is non-negative. The
 * specification also includes a logical function `foo` and an inductive definition for a generic
 * List type, modeling how a list of integers corresponds to a logical list.
 */

typedef int assert;

assert behavior = 0;

/*@ logic assert foo(assert x) = x; */

/*@ requires behavior >= 0;
    assigns behavior \from behavior;
    ensures behavior >= 0;
*/
int main () {
  behavior++;
  return 0;
}

/*@ type List<A> = Nil | Cons(A,List<A>); */

/*@ inductive model{L}(List<integer> root, List<integer>logic_list) {

case nil{L}: model(Nil,Nil);
case cons{L}: \forall List<integer> l1,List<integer>ll1;
model{L}(l1,ll1) ==> model(Cons(0,l1),Cons(0,ll1));

} */

