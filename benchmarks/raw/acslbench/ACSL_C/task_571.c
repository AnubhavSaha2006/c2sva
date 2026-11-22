/*
 * Task 571 Natural Language:
 * Write an ACSL-annotated C header file and implementations for microcontroller timing functions.
 * The code provides three functions: `sysclockInit()` for system clock initialization, `micros()`
 * that returns the current time in microseconds, and `delay(millisecs)` that pauses execution for
 * a specified number of milliseconds. The functions require valid access to the SREG register and
 * may assign values to it.
 */

#ifndef __is_included__eb42c25c_df0f_11eb_ba7e_b499badf00a1
#define __is_included__eb42c25c_df0f_11eb_ba7e_b499badf00a1 1

/* Declaration of SREG */
extern volatile unsigned char SREG;

void sysclockInit();

/*@
        requires \valid(&SREG);
        assigns SREG;
*/
unsigned long int micros();

/*@
        requires millisecs >= 0;
        requires \valid(&SREG);
        assigns SREG;
*/
void delay(unsigned long millisecs);

#endif /* __is_included__eb42c25c_df0f_11eb_ba7e_b499badf00a1 */


#include <stdio.h>

/* Dummy implementations to allow Frama-C to parse the code */

void sysclockInit() {}

/*@
        requires \valid(&SREG);
        assigns SREG;
*/
unsigned long int micros() {
  SREG = 0; /* dummy assignment */
  return 0;
}

/*@
        requires millisecs >= 0;
        requires \valid(&SREG);
        assigns SREG;
*/
void delay(unsigned long millisecs) {
  SREG = 0; /* dummy assignment */
}
