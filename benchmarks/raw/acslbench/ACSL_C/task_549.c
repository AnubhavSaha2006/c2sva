/*
 * Task 549 Natural Language:
 * Write an ACSL-annotated C program with a recursive function that searches for a value in an
 * array. The function `isPresent` takes an array `t`, its maximum index `max`, and a value `val`
 * to search for. It returns the index where `val` is found, or -1 if not found. The recursive
 * helper function `isPresentRec` searches from index `i` to `max`. The main function tests the
 * search with a sample array.
 */



/*@ requires \valid(t+(0..max));
  @ requires max>=0;
  @ requires 0<=i<=max;
  @ decreases max-i;
  @ ensures i<=\result<=max || \result==-1 ;
  @ ensures \result!=-1 ==> t[\result]==val;
  @ ensures \result==-1 ==> (\forall integer j; i<=j<=max ==> t[j]!=val);
  @ assigns \nothing;
 */
int isPresentRec(int t[], int i, int max, int val) {
//  printf("t : %d | s : %d | v : %d\n",t[0],size,val);
  if(t[i]==val) return i;
  if(max==i) return -1; // 1 de plus que max
  return isPresentRec(t, i+1, max, val);
}


/*@ requires \valid(t+(0..max));
  @ requires max>=0;
  @ ensures 0<=\result<=max || \result==-1 ;
  @ ensures \result!=-1 ==> t[\result]==val;
  @ ensures \result==-1 ==> (\forall integer i; 0<=i<=max ==> t[i]!=val);
  @ assigns \nothing;
 */
int isPresent(int t[], int max, int val) {
  return isPresentRec(t, 0, max, val);
}

/*@ assigns \nothing; */
void foo(){}

/*@ assigns \nothing; */
int main(int argc, char** argv) {
  int tab[]={10,20,33,15};
  int r=isPresent(tab, 3, 33);
  if (r==-1) foo();

//  printf("Résultat = %d\n",r);
  return 1;
}
