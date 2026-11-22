/*
 * Task 573 Natural Language:
 * Write an ACSL-annotated C program containing three functions:

1. `max3`: Finds the maximum of
 * three integers a, b, and c. The function ensures the result is the largest input value and
 * equals one of the inputs.

2. `doubleint`: Fills an array t of given size with even numbers
 * (t[i] = i*2). The function ensures the array contains consecutive even numbers and is sorted in
 * non-decreasing order.

3. `increasing`: Checks if an array t of given size is sorted in non-
 * decreasing order. The function returns 1 if sorted, 0 otherwise, with complete behavior coverage
 * for sorted and unsorted cases.
 */

/*@ requires a <= 2147483647 && a >= -2147483648;
    requires b <= 2147483647 && b >= -2147483648;
    requires c <= 2147483647 && c >= -2147483648;
    ensures \result>=a && \result>=b && \result>=c;
    ensures \result==a || \result==b || \result==c;
    ensures a>=b && a>=c ==> \result==a;
    ensures b>=a && b>=c ==> \result==b;
    ensures c>=a && c>=b ==> \result==c;
*/
int max3(int a,int b,int c) {
      int max;
      if((a >= b) && (a >= c)) {
        max = a;
      }
      else if((b >= a) && (b >= c)) {
        max = b;
      }
      else if((c >= a) && (c >= b)) {
        max = c;
      }
    return max;
}

/*@ requires 1000>size>0;
    requires \valid(t+(0..size-1));
    assigns t[0..size-1];

    ensures \forall integer i; 0<=i<size ==> t[i] == (i*2) ;
    ensures \forall integer i; 0<=i<size-1 ==> t[i]<=t[i+1];  
*/
 
void doubleint(int t[],int size) {  // void double() --> not possible since datatype!
/*@ loop invariant 0<=i<=size;
    loop invariant \forall integer j; 0<=j<i ==> t[j] == (j*2) ;
    loop invariant \forall integer j; 0<=j<i-1 ==> t[j]<=t[j+1];
    loop assigns i, t[0..size-1];
    loop variant size-i;
*/
       for(int i=0;i<size;i++) {
          t[i]=i*2;
       }
}

/*@ requires size>0;
    requires \valid(t+(0..size-1));
    assigns \nothing; 

    behavior success:
      assumes \forall integer i; 0<=i<size-1 ==> t[i]<=t[i+1];
      ensures \result==1;

    behavior failure:
      assumes \exists integer i; 0<=i<size-1 && t[i]>t[i+1];
      ensures \result==0;

    complete behaviors success, failure;
    disjoint behaviors success, failure;
*/
int increasing(int t[], int size){
/*@   loop invariant 0<=i<=size;
      loop invariant \forall integer j; 0<=j<i ==> t[j]<=t[j+1];
      loop assigns i;
      loop variant size-1-i;  
*/
      for(int i=0;i<size-1;i++) {
            if( (t[i] > t[i+1]) )  return 0;
      }
      return 1; 
}