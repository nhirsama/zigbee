#include "delay.h"
#include "display_ssd1306.h"

static void draw_page(unsigned char counter)
{
    display_clear();
    display_print_at(0u, 0u, "CC2530  Zigbee");
    display_hline(1u, 0x18u);
    display_print_at(0u, 2u, "OLED: SSD1306");
    display_print_at(0u, 3u, "SPI : P1.5 P1.6");
    display_print_at(0u, 4u, "CS/DC: P1.2 P1.7");
    display_print_at(0u, 6u, "Status: RUN ");
    display_write_char((char)('0' + counter));
    display_hline(7u, 0x81u);
}

void main(void)
{
    unsigned char counter = 0u;

    display_init();
    draw_page(counter);

    while (1) {
        delay_ms(1000u);
        counter++;
        if (counter >= 10u) counter = 0u;
        display_print_at(72u, 6u, "RUN ");
        display_write_char((char)('0' + counter));
    }
}
