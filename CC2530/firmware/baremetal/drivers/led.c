#include "led.h"

void led_init(void)
{
    /* P1.0/P1.1/P1.4 select GPIO function. */
    P1SEL &= (unsigned char)~LED_ALL;

    /* P1.0/P1.1/P1.4 output. */
    P1DIR |= LED_ALL;

    led_all_off();
}

void led_on(unsigned char mask)
{
    P1 |= (mask & LED_ALL);
}

void led_off(unsigned char mask)
{
    P1 &= (unsigned char)~(mask & LED_ALL);
}

void led_toggle(unsigned char mask)
{
    P1 ^= (mask & LED_ALL);
}

void led_all_on(void)
{
    led_on(LED_ALL);
}

void led_all_off(void)
{
    led_off(LED_ALL);
}
