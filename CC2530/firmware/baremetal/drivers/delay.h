#ifndef DELAY_H
#define DELAY_H

/* Simple busy-wait delay.  It is not calibrated; good enough for LED tests. */
void delay_cycles(unsigned int n);
void delay_ms(unsigned int ms);

#endif
