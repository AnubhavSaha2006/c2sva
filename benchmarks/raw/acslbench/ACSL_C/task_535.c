/*
 * Task 535 Natural Language:
 * Write an ACSL-annotated C function that compares two memory blocks s1 and s2 of size n bytes.
 * The function returns 0 if the blocks are identical, and a non-zero value if they differ. The
 * function requires valid read access to both memory blocks and ensures complete coverage of equal
 * and not-equal cases.
 */


#include <stdint.h>
#include <string.h>

#if 0
int
memcmp(const void *s1, const void *s2, size_t n)
{
    if (n != 0) {
        const unsigned char *p1 = s1, *p2 = s2;

        do {
            if (*p1++ != *p2++)
                return (*--p1 - *--p2);
        } while (--n != 0);
    }
    return (0);
}
#endif // 0


/*@
  requires n >= 0;
  requires n == 0 || \valid_read((char*)s1+(0..n-1));
  requires n == 0 || \valid_read((char*)s2+(0..n-1));
  assigns \nothing;
  
  behavior eq:
    assumes n >= 0 && \forall integer i; 0 <= i < n ==> ((const unsigned char*)s1)[i] == ((const unsigned char*)s2)[i];
    ensures \result == 0;

  behavior not_eq:
    assumes n > 0 && \exists integer i; 0 <= i < n && ((const unsigned char*)s1)[i] != ((const unsigned char*)s2)[i];
    ensures \result != 0;

  complete behaviors eq, not_eq;
  disjoint behaviors eq, not_eq;
*/
int memcmp(const void *s1, const void *s2, size_t n)
{
  const char *c1 = s1, *c2 = s2;
  int d = 0;


  /*@
    loop invariant 0 <= n <= \at(n, Pre);
    loop invariant (char*)s1 <= c1 <= (char*)s1 + (\at(n, Pre));
    loop invariant (char*)s2 <= c2 <= (char*)s2 + (\at(n, Pre));
    loop invariant d == 0;
    loop assigns n, c1, c2, d;
    loop variant n;
  */
  while (n) {
    /*@ assert (char*)s1 <= c1; */
    /*@ assert c1 <= (char*)s1 + (\at(n, Pre)); */
    /*@ assert (char*)s2 <= c2; */
    /*@ assert c2 <= (char*)s2 + (\at(n, Pre)); */

    d = (int)*c1++ - (int)*c2++;
    if (d)
      break;

    n--; //inserted code
  }

  return d;
}
