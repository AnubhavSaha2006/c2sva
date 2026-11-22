/*
 * Task 176 Natural Language:
 * Write ACSL-annotated C functions that verify regular expression pattern matching. The predicate
 * `matchhere` defines a recursive matching algorithm where:
- Empty regex matches any string
-
 * `x*` matches zero or more occurrences of character x (or any character if x is '.')
- `$`
 * matches end of string
- Character matches require exact equality or wildcard ('.')

Three
 * verification functions test the pattern `.*AAB` against different strings:
- `dfa_spec_aab1`:
 * Verifies "AAB" matches the pattern
- `dfa_spec_aab1_1`: Verifies "AAAB" matches the pattern  
-
 * `dfa_spec_aab0`: Verifies "AAX" does NOT match the pattern

All functions require valid null-
 * terminated strings and ensure the matching result while assigning nothing.
 */

#include<string.h>

/*@
predicate matchhere(char* r, char* s) =
 (r[0]=='\0') ||
 (r[0]!='\0' && r[1]=='*' &&
   (\exists int i; 0 <= i <= strlen(s) && matchhere(r+2, s+i) &&
   (r[0]=='.' || (\forall int j; 0 <= j < i ==> r[0]==s[j])))) ||
 (r[0]=='$' && r[1]=='\0' && s[0]=='\0') ||
 ((s[0]!='\0' && (r[0]=='.' || r[0]==s[0])) &&
  matchhere(r+1, s+1));
*/

/*@
requires strlen(r)>=0 && \valid(r+(0..strlen(r)));
requires strlen(s)>=0 && \valid(s+(0..strlen(s)));

requires r[0]=='.' && r[1]=='*' && r[2]=='A' && r[3]=='A' && r[4]=='B' && r[5]=='\0';
requires s[0]=='A' && s[1]=='A' && s[2]=='B' && s[3]=='\0';

ensures matchhere(r, s);

assigns \nothing;
*/
void dfa_spec_aab1(char* r, char* s) {}

/*@
requires strlen(r)>=0 && \valid(r+(0..strlen(r)));
requires strlen(s)>=0 && \valid(s+(0..strlen(s)));

requires r[0]=='.' && r[1]=='*' && r[2]=='A' && r[3]=='A' && r[4]=='B' && r[5]=='\0';
requires s[0]=='A' && s[1]=='A' && s[2]=='A' && s[3]=='B' && s[4]=='\0';

ensures matchhere(r, s);

assigns \nothing;
*/
void dfa_spec_aab1_1(char* r, char* s) {}

/*@
requires strlen(r)>=0 && \valid(r+(0..strlen(r)));
requires strlen(s)>=0 && \valid(s+(0..strlen(s)));

requires r[0]=='.' && r[1]=='*' && r[2]=='A' && r[3]=='A' && r[4]=='B' && r[5]=='\0';
requires s[0]=='A' && s[1]=='A' && s[2]=='X' && s[3]=='\0';

ensures !matchhere(r, s);

assigns \nothing;
*/
void dfa_spec_aab0(char* r, char* s) {}
