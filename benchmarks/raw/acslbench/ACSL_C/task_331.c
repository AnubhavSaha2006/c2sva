/*
 * Task 331 Natural Language:
 * Write an ACSL-annotated C function that calculates the third angle of a triangle given two
 * angles. The function requires both input angles to be between 0 and 180 degrees (0 ≤ first ≤
 * 180, 0 ≤ second ≤ 180) and ensures the sum of all three angles equals 180 degrees.
 */

/*@
    requires 0<=first<=180 && 0<=second<=180;
    ensures \result + first + second == 180;
*/
int last_angle(int first,int second){
    return 180 - first - second;
}
