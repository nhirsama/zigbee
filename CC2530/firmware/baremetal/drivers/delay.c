#include "delay.h"

void delay_cycles(unsigned int n)
{
    while (n--) {
        __asm
            nop
        __endasm;
    }
}

void delay_ms(unsigned int ms)
{
    while (ms--) {
        /* Approximate delay.  Adjust this constant after measuring if needed. */
        delay_cycles(1000u);
    }
}
