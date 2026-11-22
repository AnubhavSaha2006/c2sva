/*
 * Task 441 Natural Language:
 * Write an ACSL-annotated C header file that defines a structure type `t` with an integer member
 * `n` and establishes a type invariant requiring `x.n > 0`. It declares a global integer `i` and
 * defines a predicate `p` that checks if a given integer is at least `i`. The axiomatic block
 * `Bar` defines a logic integer `li` initialized to 42. The function `test` sets the global `i` to
 * 0, ensuring the postcondition `i == 0`. Another axiomatic block `Foo` introduces an abstract
 * type `foo`, a logic function `ff`, and a predicate `fp`, with an axiom relating them.
 */


typedef struct s_t { int n; } t;
/*@ type invariant inv_t(t x) = x.n > 0; */

/* @ predicate p(int x) reads x; */

int i = 42;

/*@ predicate p{Here}(int x) = x >= i; */

/*@ axiomatic Bar {
  logic integer li;
  axiom init: li == 42;
} */

/*@ ensures i == 0;
    assigns i; */
void test() { i = 0; }

/*@ axiomatic Foo {
  type foo;
  logic foo ff(foo x,char * y);
  predicate fp(foo x, foo y);
  axiom fffp: \forall foo x, char* y; fp(x,ff(x,y)) && *y == 0;
  } */