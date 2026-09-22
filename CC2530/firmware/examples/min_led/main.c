#include "led.h"

void main(void)
{
    led_init();

    /* Minimal test: turn on all LEDs and stay here. */
    led_all_on();

    while (1) {
    }
}
