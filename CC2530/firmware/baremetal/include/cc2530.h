#ifndef CC2530_H
#define CC2530_H

/*
 * Minimal CC2530 definitions for SDCC/mcs51.
 * Add registers here as drivers grow.  This is not a full TI header.
 */

/* GPIO data registers */
__sfr __at (0x80) P0;
__sfr __at (0x90) P1;
__sfr __at (0xA0) P2;

/* GPIO function select: 0 = GPIO, 1 = peripheral */
__sfr __at (0xF3) P0SEL;
__sfr __at (0xF4) P1SEL;
__sfr __at (0xF5) P2SEL;

/* GPIO direction: 0 = input, 1 = output */
__sfr __at (0xFD) P0DIR;
__sfr __at (0xFE) P1DIR;
__sfr __at (0xFF) P2DIR;

/* GPIO input mode registers.  Only declared for later drivers. */
__sfr __at (0x8F) P0INP;
__sfr __at (0xF6) P1INP;
__sfr __at (0xF7) P2INP;

/* Useful port bits */
__sbit __at (0x90) P1_0;
__sbit __at (0x91) P1_1;
__sbit __at (0x92) P1_2;
__sbit __at (0x93) P1_3;
__sbit __at (0x94) P1_4;
__sbit __at (0x95) P1_5;
__sbit __at (0x96) P1_6;
__sbit __at (0x97) P1_7;

#endif
