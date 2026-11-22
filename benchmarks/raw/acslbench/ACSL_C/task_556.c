/*
 * Task 556 Natural Language:
 * Write an ACSL-annotated C program that demonstrates function side effects and `assigns` clause
 * behavior. The program includes two functions `f()` and `g()` that both modify the global struct
 * pointer `p`'s member `a`. The `g()` function takes an integer pointer argument `p_arg` but calls
 * `f()`, which modifies the global `p->a`, showing how `assigns` clauses handle global variables
 * versus parameters.
 */



typedef struct { int a; } las;

las * p;


/*@
requires \valid(p);
assigns p->a;
@*/

void f() { p->a = 5; }

/*@
requires \valid(p);
assigns p->a;
@*/

void g(int * p_arg) { f(); }
