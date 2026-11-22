/*
 * Task 86 Natural Language:
 * Write an ACSL-annotated C function that calculates the area of a triangle given its base and
 * height. The function requires non-negative parameters (base ≥ 0 and height ≥ 0) and ensures the
 * result equals (base × height)/2. The main function demonstrates usage with base=4 and height=5,
 * verifying the result equals 10.
 */

/*@
    requires base >= 0 && height >= 0;
    ensures \result == (base * height)/2;
*/
int area(int base, int height){
    int res = (base *  height)/2;
    return res;
}

int main() {
    int a = area(4, 5);
    //@ assert a == 10;
}