#include "ugui_port.h"

#include "lcd.h"

static UG_GUI lcd_gui;

static void UGUI_Port_SetPixel(UG_S16 x, UG_S16 y, UG_COLOR color)
{
    LCD_DrawPoint((u16)x, (u16)y, (u16)color);
}

static void UGUI_Port_FillFrame(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR color)
{
    LCD_Fill((u16)x1, (u16)y1, (u16)(x2 + 1), (u16)(y2 + 1), (u16)color);
}

static void UGUI_Port_PutString(UG_S16 x, UG_S16 y, const char *str, UG_COLOR fg, UG_COLOR bg)
{
    LCD_ShowString((u16)x, (u16)y, (const u8 *)str, (u16)fg, (u16)bg, 16, 0);
}

void UGUI_Port_Init(void)
{
    UG_Init(&lcd_gui, UGUI_Port_SetPixel, LCD_W, LCD_H);
    UG_SetFillFrameCallback(&lcd_gui, UGUI_Port_FillFrame);
    UG_SetPutStringCallback(&lcd_gui, UGUI_Port_PutString);
}

UG_GUI *UGUI_Port_GetGUI(void)
{
    return &lcd_gui;
}
