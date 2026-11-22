/*
 * Task 109 Natural Language:
 * Write an ACSL-annotated C function that increments a counter `i` from 0 to 30. The loop
 * invariant ensures `i` stays within bounds (0 ≤ i ≤ 30), and the final assertion verifies `i`
 * reaches exactly 30.
 */

int main(){
    int i=0;
    /*@
        loop invariant 0 <= i <= 30;
    */
    while (i<30){
        ++i;
    }
    //@assert i==30;
   
}