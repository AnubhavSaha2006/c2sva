/*
 * Task 249 Natural Language:
 * Write an ACSL-annotated C function that initializes a specific bucket in a hash table. The
 * function takes a pointer to a Hashtbl structure and an integer index `d` as input. It requires
 * that the hash table and its data array are valid, and that the index `d` is within the valid
 * range of 0 to HASHTBL_LEN-1. The function initializes the size of the bucket at index `d` to 0
 * and also sets the overall table size to 0.
 */

#define HASHTBL_LEN 17

typedef struct {
  int b;
  int size;
} Buckets;

typedef struct {
  Buckets data[HASHTBL_LEN];
  int size;
} Hashtbl;

/*@ requires \valid(tbl);
  @ requires \valid(tbl->data+(0 .. HASHTBL_LEN - 1));
  @ requires 0 <= d < HASHTBL_LEN;
  @   
  @ assigns tbl->data[d], tbl->size; // approximation sur tbl->data[d]; si on écrit tbl->data[d].size, pas de problème
  @ */
int add(Hashtbl *tbl, int d) {
  unsigned int h = d;
  tbl->data[h].size = 0; // si on inline d, pas de probleme
  tbl->size = 0;
  return 0;
}
