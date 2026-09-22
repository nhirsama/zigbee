#ifndef LED_H
#define LED_H

#include "cc2530_min.h"

/* Board LEDs from the schematic / old code:
 * LED1 -> P1.0
 * LED2 -> P1.1
 * LED3 -> P1.4
 * Active high on this board: 1 = on, 0 = off.
 */
#define LED1_MASK      0x01u
#define LED2_MASK      0x02u
#define LED3_MASK      0x10u
#define LED_ALL_MASK   (LED1_MASK | LED2_MASK | LED3_MASK)

void led_init(void);
void led_on(unsigned char mask);
void led_off(unsigned char mask);
void led_toggle(unsigned char mask);
void led_all_on(void);
void led_all_off(void);

#endif
