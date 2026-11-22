/*
 * Task 52 Natural Language:
 * Write an ACSL-annotated C function that extracts the last character from each string in an array
 * of strings (l). The function requires all strings to be non-empty and returns a new character
 * array (rearChars) containing the last character of each input string. The parameter l_size
 * specifies the number of strings in the input array l.
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*@
  requires \forall integer i; 0 <= i < l_size ==> l[i] != NULL && strlen(l[i]) > 0;
  assigns \result;
  ensures \result != NULL;
  ensures \forall integer i; 0 <= i < l_size ==> \result[i] == l[i][strlen(l[i])-1];
*/
char* ExtractRearChars(char** l, size_t l_size) {
    char* rearChars = (char*)malloc(l_size * sizeof(char));
    
    /*@
      loop invariant 0 <= i <= l_size;
      loop invariant \forall integer k; 0 <= k < i ==> rearChars[k] == l[k][strlen(l[k])-1];
      loop assigns i, rearChars[0..l_size-1];
      loop variant l_size - i;
    */
    for (size_t i = 0; i < l_size; ++i) {
        size_t str_len = strlen(l[i]);
        rearChars[i] = l[i][str_len-1];
    }

    return rearChars;
}