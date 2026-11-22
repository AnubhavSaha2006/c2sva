/*
 * Task 416 Natural Language:
 * Write an ACSL-annotated C function that parses the first character of a string. If the first
 * character is a digit ('0'-'9'), the function returns its integer value (0-9). If the first
 * character is not a digit or the string is empty, the function returns -1. The function requires
 * a valid, null-terminated string and ensures the result is either -1 or an integer between 0 and
 * 9.
 */

#include <limits.h>
#include <string.h>
/*@
requires ((strlen(x0)>=0) &&
\valid(x0+(0..strlen(x0))));
assigns \nothing;
ensures ((\result==-1) || ((0<=\result) &&
(\result<=9)));
*/
int p(char  * x0) {
  int x8 = -1;
  int x9 = 1/*true*/;
  char x10 = '\0';
  char  *x11 = 0/*null*/;
  char x2 = x0[0];
  int x3 = x2 == '\0';
  if (x3) {
    x11 = x0;
  } else {
    int x4 = x2 >= '0';
    int x6;
    if (x4) {
      int x5 = x2 <= '9';
      x6 = x5;
    } else {
      x6 = 0/*false*/;
    }
    if (x6) {
      x9 = 0/*false*/;
      x10 = x2;
      char  *x7 = x0+1;
      x11 = x7;
    } else {
      x11 = x0;
    }
  }
  int x23 = x9;
  if (x23) {
    char  *x24 = x11;
  } else {
    char x26 = x10;
    char  *x28 = x11;
    char x27 = x26 - '0';
    x8 = x27;
  }
  int x32 = x8;
  return x32;
}
