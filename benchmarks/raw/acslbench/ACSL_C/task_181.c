/*
 * Task 181 Natural Language:
 * Write an ACSL-annotated C program that demonstrates pointer aliasing and structure access. The
 * program defines nested structures S and T, where T contains pointers to S. Function f requires
 * valid pointers to T and its first element array, and modifies t1[0] of the pointed S structure.
 * The main function creates instances, links them, and calls f to demonstrate valid memory access
 * patterns with specific zone mappings for global and local variables.
 */


struct S {
  int t1[2];
  int t2[2];
};

struct T {
  struct S *t[2];
};

/*@ requires \valid(s) && \valid(s->t[0]);
  @*/
void f(struct T *s) {
  s->t[0]->t1[0] = 1;
}

int main(struct T s, struct S a) {
  s.t[0] = &a;
  f(&s);
  return 0;
}

/* on veut :

zones globales :

 Zone 0: {s.t[0]; };
 Zone 1: {a.t1[0]; };

zones locales :

 f:
 Zone 4: { *s; }
 Zone 5: { s->t[0]; }

Appels:

 f(..) ligne 25: zone 4 -> zone 0, zone 5 -> zone 1

*/
