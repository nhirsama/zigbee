#ifndef LED_H
#define LED_H

#include "board.h"

#define LED1        BOARD_LED1_MASK
#define LED2        BOARD_LED2_MASK
#define LED3        BOARD_LED3_MASK
#define LED_ALL     BOARD_LED_ALL_MASK

void led_init(void);
void led_on(unsigned char mask);
void led_off(unsigned char mask);
void led_toggle(unsigned char mask);
void led_all_on(void);
void led_all_off(void);

#endif
