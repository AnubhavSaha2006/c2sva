/*
 * Task 473 Natural Language:
 * Write an ACSL-annotated C function that finds the maximum and minimum values in an integer
 * array. The function takes a pointer to an array `a` and its `size` as input. It requires that
 * the array size is greater than 0 and that the array is valid for reading in the range from index
 * 0 to `size-1`. The function returns a structure containing the maximum (`max`) and minimum
 * (`min`) values found in the array. The postcondition ensures that for every element in the
 * array, it is less than or equal to the returned maximum and greater than or equal to the
 * returned minimum.
 */

struct MaxAndMin{
                  int max;
                  int min;
       };
       /*@ requires size > 0 && \valid_read(a+(0..size-1));
           ensures \forall integer i ; 0 <= i < size ==> ( a[i] <= \result.max && \result.min <= a[i]  ) ;
            */
       struct MaxAndMin findMaxAndMin(int* a,int size){
                   struct MaxAndMin s;
                   struct MaxAndMin* p = &s;
                   int** q = &a;
                   p->min = (*q)[0];
                   p->max = (*q)[0];
                  int i = 0;
                 /*@ loop assigns i, s.max, s.min;
                     loop invariant *p==s && *q==a && size >0 && 
			(i<=size && (i>=0 && (\forall integer j ;0 <= j < i ==> ( a[j] <=s.max && s.min <=a[j]))));
			
                 loop variant size - i;
                 */
                while (i < size){
                           if (p->max < (*q)[i])
                                      p->max = (*q)[i];
                           if (p->min > (*q)[i])
                                      p->min = (*q) [i];
                           i = i + 1;
                 }
                 return s;
      }