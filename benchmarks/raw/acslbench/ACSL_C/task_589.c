/*
 * Task 589 Natural Language:
 * Write an ACSL-annotated C function that performs binary search for value v in a sorted array t
 * of size n. The function returns the index of v if found (between 0 and n-1), or -1 if not found.
 * The array must be valid and non-empty (n ≥ 0). The implementation uses the lemma that for any
 * integers x and y where x ≤ y, the mean (x+y)/2 lies between x and y.
 */

/*@ lemma mean : \forall integer x, y; x <= y ==> x <= (x+y)/2 <= y; */
/*@ requires n >= 0 && \valid(t+(0..n-1));
@ ensures -1 <= \result <= n-1;
@ assigns \nothing;
@*/
int binary_search4(int* t, int n, int v) {
int l = 0, u = n-1, p = -1;
/*@ loop invariant
@ 0 <= l && u < n && -1 <= p <= n-1;
@ loop invariant \forall integer i; l <= i <= u ==> \valid_read(t+i);
@ loop assigns l, u, p;
@ loop invariant l <= u+1;
@ loop variant u - l;
@*/
while (l <= u) {
int m = l + (u - l) / 2;
if (t[m] < v)
l = m + 1;
else if (t[m] > v)
u = m - 1;
else {
p = m; break;
}
}
return p;
}