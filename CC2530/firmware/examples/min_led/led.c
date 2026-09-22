#include "led.h"

void led_init(void)
{
    /* Select GPIO function for P1.0/P1.1/P1.4. */
    P1SEL &= (unsigned char)~LED_ALL_MASK;

    /* Configure P1.0/P1.1/P1.4 as output. */
    P1DIR |= LED_ALL_MASK;
}

void led_on(unsigned char mask)
{
    P1 |= (mask & LED_ALL_MASK);
}

void led_off(unsigned char mask)
{
    P1 &= (unsigned char)~(mask & LED_ALL_MASK);
}

void led_toggle(unsigned char mask)
{
    P1 ^= (mask & LED_ALL_MASK);
}

void led_all_on(void)
{
    led_on(LED_ALL_MASK);
}

void led_all_off(void)
{
    led_off(LED_ALL_MASK);
}
