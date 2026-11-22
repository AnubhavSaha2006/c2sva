/*
 * Task 579 Natural Language:
 * Write an ACSL-annotated C function that initializes a MyObject structure. The function requires
 * a valid pointer to a MyObject where the id field is initially 0. It assigns values to the id
 * field and all elements of the myTab array (size 5). The function ensures that after execution,
 * the id remains 0 and all elements in myTab array are set to 0.
 */

/*@ ghost const int MYOBJECT_MYTAB_SIZE = 5; */
#define MYOBJECT_MYTAB_SIZE 5

typedef struct MyObject{
	int id;
	int myTab[MYOBJECT_MYTAB_SIZE];
}MyObject;


/*@ requires \valid(self);
    requires Inv1 : self->id == 0;

    assigns self->id, self->myTab[0..MYOBJECT_MYTAB_SIZE-1];

    ensures self->id == 0;
    ensures \forall integer it_x; 0 <= it_x < MYOBJECT_MYTAB_SIZE ==> self->myTab[it_x] == 0;
*/
void test(MyObject* self){
	self->id = 0;
	/*@ loop invariant 0 <= i <= MYOBJECT_MYTAB_SIZE;
	    loop invariant \forall integer j; 0 <= j < i ==> self->myTab[j] == 0;
	    loop assigns i, self->myTab[0..MYOBJECT_MYTAB_SIZE-1];
	    loop variant MYOBJECT_MYTAB_SIZE - i;
	*/
	for (int i=0; i<MYOBJECT_MYTAB_SIZE;i++){
		self->myTab[i] = 0;
	}
}