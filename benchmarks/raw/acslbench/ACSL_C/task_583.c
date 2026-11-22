/*
 * Task 583 Natural Language:
 * Write three ACSL-annotated C functions and a main function. Function `f` takes an integer `x`,
 * sets it to 0, and returns 0. Function `g` takes an integer `x` and returns the value of global
 * variable `Y` (which is 1). Function `h` takes no parameters and returns 0. The main function
 * returns 0. All functions are specified to assign no memory locations.
 */


/*@ ensures \result == 0;
    assigns \nothing; */
int f(int x)
{
  x = 0;
  return x;
}

int Y = 1;

/*@ ensures \result == Y;
    assigns \nothing; */
int g(int x)
{
  return Y;
}

/*@ ensures \result == 0;
    assigns \nothing; */
int h(void)
{
  int __retres;
  __retres = 0;
  return __retres;
}

/*@ assigns \nothing; */
int main(void)
{
  int __retres;
  __retres = 0;
  return __retres;
}
