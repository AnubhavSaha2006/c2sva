/*
 * Task 338 Natural Language:
 * Write an ACSL-annotated C program with functions that manipulate strings. The functions `a` and
 * `b` take a string `s` and ensure its length and content remain unchanged after modification. The
 * functions `an` and `bn` take a string `s` and an integer `n`, assigning values to the first `n`
 * elements of the string. All functions require a valid, null-terminated string as input.
 */

#include "string.h"

/*@
    requires valid_string(s);
    assigns s[0..(strlen(s) - 2)];
    ensures strlen(s) ≡ \old(strlen(s));
    ensures s ≡ \old(s);
*/
void a(char* s);

/*@
    requires valid_string(s);
    assigns s[0..strlen(\old(s))];
    ensures strlen(s) ≡ strlen(s);
    ensures s ≡ \old(s);
*/
void b(char *s) {
    int x = 0;
    a(s);
    //@ assert x ≡ 0;
}



/*@
    requires valid_string(s);
    assigns s[0..n];
*/
void an(char* s, int n);

/*@
    requires valid_string(s);
    assigns s[0..n];
*/
void bn(char *s, int n) {
    an(s, n);
}
