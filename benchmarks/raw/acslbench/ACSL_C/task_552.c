/*
 * Task 552 Natural Language:
 * Write an ACSL-annotated C program that demonstrates the use of the `\at` operator with logical
 * variables. The program includes assertions that check properties at specific program labels (L,
 * Q, K, G) using existential and universal quantifiers, logical expressions, and nested let-
 * bindings. The main function tests various predicates and terms involving integer arithmetic and
 * label-specific variable values.
 */



/*@ assigns \nothing; */
void f(int *t) {}

/*@ assigns \nothing; */
void g() {
  int m;
  m = 8;
  Q: ;
  m = 10;
  /*@ assert \exists integer w; 3 <= w < 6 && \at(m + w == 12, Q); */ ;
}

/*@ assigns \nothing; */
int main(void) {
  int n;
  n = 7;
  L: ;
  n = 9;
  K: ;
  n = 666;

  // Predicates:
  /*@ assert \let i = 3; \at(n + i == 10, L); */ ;
  /*@ assert \exists integer j; 2 <= j < 5 && \at(n + j == 11, L); */ ;
  /*@ assert
      \let k = -7;
      \exists integer u; 9 <= u < 21 &&
      \forall integer v; -5 < v <= 6 ==>      \at((u > 0 ? n + k : u + v) > 0, K); */ ;

  // Terms:
  /*@ assert \let i = 3; \at(n + i, L) == 10; */ ;
  unsigned int m = 3;
  G: ;
  m = -3;
  /*@ assert \exists integer k; -9 < k < 0 && \at(m + k, G) == 0; */ ;
  /*@ assert
      \exists integer u; 9 <= u < 21 &&
      \forall integer v; -5 < v <= (u < 15 ? u + 6 : 3) ==>        \at(n + u + v > 0, K); */ ;

  // Function calls:
  int t[5] = {9, 12, 12, 12, -4};
  f(t);
  g();

  // Name capturing
  /*@ assert
         \exists integer u; 10 <= u < 20
      && \exists integer v; -10 < v <= -5  + (\let u = -2; u) // another u
      && \exists integer w; 10 < w <= 20
      && \at(n - u +
          (\let u = 42; u) // yet another u
          + v + w > 0, K); */ ;

  return 0;
}