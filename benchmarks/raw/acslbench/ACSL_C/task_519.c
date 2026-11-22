/*
 * Task 519 Natural Language:
 * Write an ACSL-annotated C function that calculates the length of the longest common prefix
 * between two suffixes of an array `a` of size `N`, starting at indices `x` and `y`. The function
 * requires valid array indices and ensures a non-negative result.
 */

/*@
    requires \valid_read(a+(0..N-1)) && 0 <= x && x < N && 0 <= y && y < N;
    ensures \result >= 0;
*/
int lcp(int a[], int N, int x, int y)
{
    int l = 0;
    /*@ loop invariant 0 <= l <= N;
        loop invariant x + l <= N && y + l <= N;
        loop assigns l;
        loop variant N - l;
    */
    for (;;) {
        if (!(x + l < N && y + l < N && a[x+l] == a[y+l])) {
            break;
        }
        l++;
    }
    return l;
}