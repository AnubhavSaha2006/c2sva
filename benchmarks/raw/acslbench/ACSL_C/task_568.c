/*
 * Task 568 Natural Language:
 * Write an ACSL-annotated C function that takes a character array pointer `a` and initializes it
 * to the string "abcabc" with null termination. The function requires a valid array of at least 7
 * elements and ensures the array contains the exact sequence 'a','b','c','a','b','c','\0'.
 */

/*@ requires \valid(a + (0..6));
    ensures  \valid_read(a + (0..6)) && a[6] == '\0' && 
             (a[0] == 'a') && (a[1] == 'b') && (a[2] == 'c') && 
             (a[3] == 'a') && (a[4] == 'b') && (a[5] == 'c');
*/
char* strtest(char* a){
    a[0] = 'a'; a[1] = 'b'; a[2] = 'c'; a[3] = 'a'; a[4] = 'b'; a[5] = 'c'; a[6] = '\0';
    return a;
}