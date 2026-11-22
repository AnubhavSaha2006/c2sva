/*
 * Task 553 Natural Language:
 * Write an ACSL-annotated C function that calculates the area of a triangle given its base and
 * height. The function requires that both base and height are non-negative and that their product
 * does not cause integer overflow. The function ensures the result equals (base * height) / 2.
 */

/*@
requires base >= 0 && height >= 0 && base <= 2147483647 / height;
ensures \result == (base * height)/2;
assigns \nothing;
*/
int area(int base, int height){
    int res = (base *  height)/2;
    return res;
}
/*@
 assigns \nothing;
*/
int main() {
    int a = area(4, 5);
    //@ assert a == 10;
}