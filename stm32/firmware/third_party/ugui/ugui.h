/* Minimal UGUI-compatible drawing subset for this STM32 project. */
#ifndef UGUI_H_
#define UGUI_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int16_t UG_S16;
typedef uint16_t UG_U16;
typedef uint8_t UG_U8;
typedef uint16_t UG_COLOR;
typedef uint8_t UG_BOOL;

typedef void (*UG_DEVICE_SET_PIXEL)(UG_S16 x, UG_S16 y, UG_COLOR color);
typedef void (*UG_DEVICE_FILL_FRAME)(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR color);
typedef void (*UG_DEVICE_PUT_STRING)(UG_S16 x, UG_S16 y, const char *str, UG_COLOR fg, UG_COLOR bg);

typedef struct {
    UG_S16 x_dim;
    UG_S16 y_dim;
    UG_COLOR fore_color;
    UG_COLOR back_color;
    UG_DEVICE_SET_PIXEL set_pixel;
    UG_DEVICE_FILL_FRAME fill_frame;
    UG_DEVICE_PUT_STRING put_string;
} UG_GUI;

#define UG_TRUE  ((UG_BOOL)1u)
#define UG_FALSE ((UG_BOOL)0u)

#define UG_RGB565(r, g, b) \
    ((UG_COLOR)(((((uint16_t)(r)) & 0xF8u) << 8) | ((((uint16_t)(g)) & 0xFCu) << 3) | ((((uint16_t)(b)) & 0xF8u) >> 3)))

#define UG_COLOR_BLACK      ((UG_COLOR)0x0000u)
#define UG_COLOR_WHITE      ((UG_COLOR)0xFFFFu)
#define UG_COLOR_RED        ((UG_COLOR)0xF800u)
#define UG_COLOR_GREEN      ((UG_COLOR)0x07E0u)
#define UG_COLOR_BLUE       ((UG_COLOR)0x001Fu)
#define UG_COLOR_YELLOW     ((UG_COLOR)0xFFE0u)
#define UG_COLOR_CYAN       ((UG_COLOR)0x07FFu)
#define UG_COLOR_MAGENTA    ((UG_COLOR)0xF81Fu)
#define UG_COLOR_GRAY       ((UG_COLOR)0x8410u)
#define UG_COLOR_DARK_GRAY  ((UG_COLOR)0x4208u)
#define UG_COLOR_LIGHT_GRAY ((UG_COLOR)0xC618u)
#define UG_COLOR_NAVY       ((UG_COLOR)0x000Fu)
#define UG_COLOR_ORANGE     ((UG_COLOR)0xFD20u)
#define UG_COLOR_PANEL      UG_RGB565(23, 31, 44)
#define UG_COLOR_CARD       UG_RGB565(34, 45, 63)
#define UG_COLOR_CARD2      UG_RGB565(44, 58, 82)
#define UG_COLOR_ACCENT     UG_RGB565(41, 128, 255)
#define UG_COLOR_OK         UG_RGB565(26, 188, 112)
#define UG_COLOR_WARN       UG_RGB565(255, 184, 51)
#define UG_COLOR_TEXT       UG_RGB565(235, 241, 255)
#define UG_COLOR_MUTED      UG_RGB565(148, 163, 184)

void UG_Init(UG_GUI *gui, UG_DEVICE_SET_PIXEL set_pixel, UG_S16 x_dim, UG_S16 y_dim);
void UG_SelectGUI(UG_GUI *gui);
UG_GUI *UG_GetGUI(void);
void UG_SetFillFrameCallback(UG_GUI *gui, UG_DEVICE_FILL_FRAME fill_frame);
void UG_SetPutStringCallback(UG_GUI *gui, UG_DEVICE_PUT_STRING put_string);
void UG_SetForecolor(UG_COLOR color);
void UG_SetBackcolor(UG_COLOR color);
UG_COLOR UG_GetForecolor(void);
UG_COLOR UG_GetBackcolor(void);
UG_S16 UG_GetXDim(void);
UG_S16 UG_GetYDim(void);

void UG_DrawPixel(UG_S16 x, UG_S16 y, UG_COLOR color);
void UG_FillScreen(UG_COLOR color);
void UG_FillFrame(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR color);
void UG_DrawFrame(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR color);
void UG_DrawLine(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR color);
void UG_DrawHLine(UG_S16 x1, UG_S16 x2, UG_S16 y, UG_COLOR color);
void UG_DrawVLine(UG_S16 x, UG_S16 y1, UG_S16 y2, UG_COLOR color);
void UG_PutString(UG_S16 x, UG_S16 y, const char *str);
void UG_PutStringColor(UG_S16 x, UG_S16 y, const char *str, UG_COLOR fg, UG_COLOR bg);

void UG_DrawButton(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2,
                   const char *text, UG_BOOL selected, UG_BOOL active);
void UG_DrawToggle(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2,
                   const char *label, UG_BOOL selected, UG_BOOL on);
void UG_DrawProgress(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2,
                     UG_U8 percent, UG_COLOR fill_color);

#ifdef __cplusplus
}
#endif

#endif /* UGUI_H_ */
