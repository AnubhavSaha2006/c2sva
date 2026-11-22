/*
 * Task 503 Natural Language:
 * Write an ACSL-annotated C program that ensures a pointer parameter `p` is valid. The program
 * includes a function `f` that also ensures its pointer parameter `q` is valid. The main function
 * initializes a local variable, sets an enumeration value to `OK`, and calls function `f` with the
 * pointer `p`.
 */

/* run.config
   OPT: -load-module lib/plugins/Obfuscator -obfuscate -journal-disable
*/

/*@ ensures \valid(q); 
 */
int f(int *q) ;

#define LV X_9999999999999999999999999999999999999999999999999999
int global_LV;
enum { OK = 1,
       NOT_OK = 0 } e ;

/*@ ensures \valid(p);
 */
void main (int * p) {
int LV = 0;
 e = OK ;
 f(p);
}