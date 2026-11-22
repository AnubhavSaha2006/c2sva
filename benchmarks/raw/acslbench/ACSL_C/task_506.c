/*
 * Task 506 Natural Language:
 * Write an ACSL-annotated C program that includes function specifications, lemmas, and type
 * definitions. The function `f` ensures that the floating-point value 1.0 equals the integer 1
 * when compared, and a lemma states that 1.0 equals the float-cast of 1. The program also defines
 * integer type `T`, array type `T4`, pointer type `T_PTR`, and a complex pointer array type
 * `T_PTR_T4` with constant instances.
 */

/* bts 60: 1 should be lifted to a real number implicitely.  */

/*@ ensures 1.0 == 1; 
    assigns \nothing;*/
void f();

/*@ lemma foo: 1.0 == (float)1; */


void f() {
 double B;

}


typedef int T, T4[4], *T_PTR;
const T X, Tab[4];
typedef T_PTR T_PTR_T4[4];
const T_PTR_T4  Tab_Ptr = { &X, &X, &X, &X};
