/*
 * Task 49 Natural Language:
 * Write an ACSL-annotated C function that checks if a string `s` starts and ends with the same
 * character. The function requires the string to be non-empty and returns true if the first and
 * last characters match.
 */

#include <stdbool.h>
#include <string.h>

/*@
  requires strlen(s) > 0;
  assigns \nothing;
  ensures \result <==> (s[0] == s[strlen(s)-1]);
*/
bool StartAndEndWithSameChar(const char* s) {
    size_t length = strlen(s);
    bool result = (s[0] == s[length - 1]);
    return result;
}