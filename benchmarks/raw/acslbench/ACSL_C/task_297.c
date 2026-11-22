/*
 * Task 297 Natural Language:
 * Write an ACSL-annotated C function that calculates the length of a null-terminated string s, but
 * counts at most maxlen characters. The function returns the actual string length if a null
 * terminator is found within maxlen characters, otherwise returns maxlen. Requires maxlen ≥ 0 and
 * valid memory access for s[0..maxlen-1].
 */


#include <stdint.h>
#include <string.h>

/*
u32 strnlen(const char * s, u32 count){
	const char *sc;

	for (sc = s; count-- && *sc != '\0'; ++sc);
	return (u32)(sc - s);
}*/

/*@
  requires maxlen >= 0;
  requires \valid(s+(0..maxlen-1));
  assigns \nothing;
  behavior bigger:
    assumes \forall integer i; 0 <= i < maxlen ==> s[i] != 0;
    ensures \result == maxlen;

  behavior smaller:
    assumes \exists integer i; 0 <= i < maxlen && s[i] == 0;
    ensures \result <= maxlen;
  complete behaviors;
  disjoint behaviors;
*/
size_t strnlen(const char *s, size_t maxlen)
{
  const char *ss = s;

  /* Important: the maxlen test must precede the reference through ss;
     since the byte beyond the maximum may segfault */

  /*@
    loop invariant 0 <= maxlen <= \at(maxlen,Pre);
    loop invariant \forall integer i; 0 <= i < (\at(maxlen, Pre) - maxlen) ==> s[i] != 0;
    loop invariant ss == s+(\at(maxlen, Pre) - maxlen);
    loop invariant s <= ss <= s+\at(maxlen, Pre);
    loop assigns maxlen, ss;
    loop variant maxlen;
  */
  while ((maxlen > 0) && *ss) {
    ss++;
    maxlen--;
  }
  return ss - s;
}

