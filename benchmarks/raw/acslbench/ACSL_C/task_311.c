/*
 * Task 311 Natural Language:
 * Write an ACSL-annotated C program that defines two functions, `f` and `g`, each taking a pointer
 * to a struct `_list` (aliased as `list`). Both functions require that the input pointer `p` is
 * valid (points to a properly allocated memory location).
 */

typedef struct _list {
  int i;
} *list;

/*@ requires \valid(p); */
void f(list p) {}

/*@ requires \valid(p); */
void g(struct _list* p) {}
