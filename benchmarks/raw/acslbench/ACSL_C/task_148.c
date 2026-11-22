/*
 * Task 148 Natural Language:
 * Write an ACSL-annotated C function that finds the maximum of two integers. The function takes
 * three integer pointers: `r` (for storing the result), `i`, and `j`. It requires that `i` and `j`
 * point to valid memory locations, and if `r` is not null, it must also point to a valid location.
 * If `r` is null, the function returns -1 without modifying any memory. If `r` is valid, it stores
 * the maximum of `*i` and `*j` in `*r` and returns 0.
 */

/*@ requires \valid(i) && \valid(j);

@ requires r == \null || \valid(r);

@ assigns *r;

@ behavior zero:

@ assumes r == \null;

@ assigns \nothing;

@ ensures \result == -1;

@ behavior normal:

@ assumes \valid(r);

@ assigns *r;

@ ensures *r == ((*i < *j) ? *j : *i);

@ ensures \result == 0;

@*/

int max2(int *r, int* i, int* j) {

if (!r) return -1;

*r = (*i < *j) ? *j : *i;



return 0;

}



