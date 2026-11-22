/*
 * Task 512 Natural Language:
 * Write an ACSL-annotated C function that searches for a value `val` in an integer array `t` of
 * size `max+1`. The function returns the index of the first occurrence of `val` if found (0 ≤
 * index ≤ max), or returns -1 if `val` is not present in the array. The function requires a valid
 * array `t` with indices from 0 to `max`, a non-negative `max` value less than 2147483647, and
 * ensures the result is either a valid index or -1 with corresponding correctness conditions.
 */



/*@ requires \valid(t+(0..max));
  @ requires max>=0;
  @ requires max < 2147483647;
  @ ensures 0<=\result && \result<=max || \result==-1 ;
  @ ensures 0<=\result && \result<=max ==> t[\result]==val;
  @ ensures \result==-1 ==> (\forall integer j; 0<=j<=max ==> t[j]!=val);
  @ assigns \nothing;
 */
int isPresent(int t[], int max, int val) {  
  int i=0;
  /*@ loop invariant inv :
    @      0<=i<=max+1
    @   && \valid(t+(0..max))
    @   && max>=0
    @   && (\forall integer j; 0<=j<i ==> t[j]!=val);
    @  loop assigns i;
    @  loop variant v : max-i ;
   */
  while (i<max+1 && t[i]!=val) {
    i++;
  }
  if(i<=max && t[i]==val) return i;
  return -1;
}

/*@ assigns \nothing; */
void foo(){}

/*@ assigns \nothing; */
int main(int argc, char** argv) {
  int tab[]={10,20,33,15};
  int r=isPresent(tab, 3, 33);

  if (r==-1) foo();
  
  return 1;
}