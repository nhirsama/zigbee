#ifndef CC2530_MIN_H
#define CC2530_MIN_H

/* Minimal CC2530 SFR definitions for SDCC/mcs51. */
__sfr __at (0x90) P1;      /* Port 1 data register */
__sfr __at (0xF4) P1SEL;   /* Port 1 function select: 0 = GPIO */
__sfr __at (0xFE) P1DIR;   /* Port 1 direction: 1 = output */

#endif
