/*
 * Task 425 Natural Language:
 * Write an ACSL-annotated C function that determines if three positive integers x, y, and z can
 * form a triangle and, if so, what type. The function returns 1 only if the sides form an
 * equilateral triangle (all sides equal). It returns 0 for all other cases, including when the
 * sides cannot form any triangle, form an isosceles triangle (exactly two sides equal), or form a
 * scalene triangle (all sides different). The inputs must be positive integers (x ≥ 1, y ≥ 1, z ≥
 * 1) and their sums must not cause integer overflow.
 */

#include<limits.h>

/*@ requires x >= 1;
 requires y >= 1;
 requires z>= 1;
 requires x<=INT_MAX && y<=INT_MAX && z<=INT_MAX;
 requires x+y<=INT_MAX && y+z<=INT_MAX && z+x<=INT_MAX;
 ensures (x+y <= z || y+z <=x || x+z <=y) ==> \result == 0;
 ensures x+y > z && y+z >x && x+z >y && x == y && y == z ==> \result==1;
 ensures x+y > z && y+z >x && x+z >y && (x != y || y != z) ==> \result==0;
 assigns \nothing;
*/
int triangle(int x,int y,int z)
{

 if(x+y > z && y+z >x && x+z >y)
 {
     if(x == y && y == z){
     return 1;
     }
     else if(x == y || y == z || x==z){
     return 0;
     }
     else if((x!=y)&&(y!=z)&&(z!=x)) {
     return 0;
     }
   }
    return 0;
}

int main()
{
 int t = triangle(4,4,4);
 return 0;
}