#ifndef BOARD_H
#define BOARD_H

#include "cc2530.h"

/*
 * Xinyingda / E18-MS1-PCB CC2530 board pins found in the schematic.
 * Keep all board-specific pin mapping here.
 */

/* LEDs: active high */
#define BOARD_LED1_MASK       0x01u  /* P1.0 */
#define BOARD_LED2_MASK       0x02u  /* P1.1 */
#define BOARD_LED3_MASK       0x10u  /* P1.4 */
#define BOARD_LED_ALL_MASK    (BOARD_LED1_MASK | BOARD_LED2_MASK | BOARD_LED3_MASK)

/* Other exposed pins on this board, for future drivers. */
#define BOARD_DS18B20_MASK    0x08u  /* P1.3 */

/*
 * 7-pin display header from the schematic:
 *   D0/SCLK -> P1.5
 *   D1/MOSI -> P1.6
 *   RES     -> board RESET net
 *   DC      -> P1.7
 *   CS      -> P1.2
 *
 * P1.7 is also the module pin normally labelled MISO on the E18-MS1-PCB,
 * but on the display header it is used as D/C.
 */
#define BOARD_LCD_CS_MASK     0x04u  /* P1.2 */
#define BOARD_LCD_SCLK_MASK   0x20u  /* P1.5 */
#define BOARD_LCD_MOSI_MASK   0x40u  /* P1.6 */
#define BOARD_LCD_DC_MASK     0x80u  /* P1.7 */

#define BOARD_SPI_SCLK_MASK   BOARD_LCD_SCLK_MASK
#define BOARD_SPI_MOSI_MASK   BOARD_LCD_MOSI_MASK
#define BOARD_SPI_MISO_MASK   0x80u  /* P1.7, shared with display D/C */

void board_init(void);

#endif
