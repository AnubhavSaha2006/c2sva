/*
 * Task 159 Natural Language:
 * Write an ACSL-annotated C function that checks if a given character is an alphabet letter. The
 * function returns 1 if the character is between 'a' and 'z' or between 'A' and 'Z', and returns 0
 * otherwise. The input is a single character of type char.
 */

/*@
	ensures (('a'<=c && c<='z') || ('A'<=c && c<='Z')) ==> \result==1;
	ensures !(('a'<=c && c<='z') || ('A'<=c && c<='Z'))==> \result==0;
*/
int alphabet_letter(char c){
	if( ('a'<= c && c <='z') || ('A' <= c && c <='Z') ) return 1;
	else return 0;
}
int main(){
	int ch ;
	ch = alphabet_letter('x') ;
	//@ assert ch==1 ;
	ch = alphabet_letter('H') ;
	//@ assert ch==1 ;
	ch = alphabet_letter(' ') ;
	//@ assert !ch ;
}
