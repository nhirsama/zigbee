/* Minimal UGUI-compatible drawing subset for this STM32 project. */
#include "ugui.h"

#include <stddef.h>
#include <string.h>

static UG_GUI *active_gui = 0;

static void ug_swap_s16(UG_S16 *a, UG_S16 *b)
{
    UG_S16 t = *a;
    *a = *b;
    *b = t;
}

static UG_BOOL ug_clip_point(UG_S16 *x, UG_S16 *y)
{
    if (active_gui == 0 || active_gui->set_pixel == 0) {
        return UG_FALSE;
    }
    if (*x < 0 || *y < 0 || *x >= active_gui->x_dim || *y >= active_gui->y_dim) {
        return UG_FALSE;
    }
    return UG_TRUE;
}

static UG_BOOL ug_clip_frame(UG_S16 *x1, UG_S16 *y1, UG_S16 *x2, UG_S16 *y2)
{
    if (active_gui == 0 || active_gui->set_pixel == 0) {
        return UG_FALSE;
    }
    if (*x1 > *x2) {
        ug_swap_s16(x1, x2);
    }
    if (*y1 > *y2) {
        ug_swap_s16(y1, y2);
    }
    if (*x2 < 0 || *y2 < 0 || *x1 >= active_gui->x_dim || *y1 >= active_gui->y_dim) {
        return UG_FALSE;
    }
    if (*x1 < 0) {
        *x1 = 0;
    }
    if (*y1 < 0) {
        *y1 = 0;
    }
    if (*x2 >= active_gui->x_dim) {
        *x2 = active_gui->x_dim - 1;
    }
    if (*y2 >= active_gui->y_dim) {
        *y2 = active_gui->y_dim - 1;
    }
    return UG_TRUE;
}

void UG_Init(UG_GUI *gui, UG_DEVICE_SET_PIXEL set_pixel, UG_S16 x_dim, UG_S16 y_dim)
{
    if (gui == 0) {
        return;
    }
    gui->x_dim = x_dim;
    gui->y_dim = y_dim;
    gui->fore_color = UG_COLOR_TEXT;
    gui->back_color = UG_COLOR_PANEL;
    gui->set_pixel = set_pixel;
    gui->fill_frame = 0;
    gui->put_string = 0;
    UG_SelectGUI(gui);
}

void UG_SelectGUI(UG_GUI *gui)
{
    active_gui = gui;
}

UG_GUI *UG_GetGUI(void)
{
    return active_gui;
}

void UG_SetFillFrameCallback(UG_GUI *gui, UG_DEVICE_FILL_FRAME fill_frame)
{
    if (gui != 0) {
        gui->fill_frame = fill_frame;
    }
}

void UG_SetPutStringCallback(UG_GUI *gui, UG_DEVICE_PUT_STRING put_string)
{
    if (gui != 0) {
        gui->put_string = put_string;
    }
}

void UG_SetForecolor(UG_COLOR color)
{
    if (active_gui != 0) {
        active_gui->fore_color = color;
    }
}

void UG_SetBackcolor(UG_COLOR color)
{
    if (active_gui != 0) {
        active_gui->back_color = color;
    }
}

UG_COLOR UG_GetForecolor(void)
{
    return (active_gui != 0) ? active_gui->fore_color : UG_COLOR_WHITE;
}

UG_COLOR UG_GetBackcolor(void)
{
    return (active_gui != 0) ? active_gui->back_color : UG_COLOR_BLACK;
}

UG_S16 UG_GetXDim(void)
{
    return (active_gui != 0) ? active_gui->x_dim : 0;
}

UG_S16 UG_GetYDim(void)
{
    return (active_gui != 0) ? active_gui->y_dim : 0;
}

void UG_DrawPixel(UG_S16 x, UG_S16 y, UG_COLOR color)
{
    if (ug_clip_point(&x, &y) == UG_FALSE) {
        return;
    }
    active_gui->set_pixel(x, y, color);
}

void UG_FillScreen(UG_COLOR color)
{
    if (active_gui == 0) {
        return;
    }
    UG_FillFrame(0, 0, active_gui->x_dim - 1, active_gui->y_dim - 1, color);
}

void UG_FillFrame(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR color)
{
    UG_S16 x;
    UG_S16 y;

    if (ug_clip_frame(&x1, &y1, &x2, &y2) == UG_FALSE) {
        return;
    }

    if (active_gui->fill_frame != 0) {
        active_gui->fill_frame(x1, y1, x2, y2, color);
        return;
    }

    for (y = y1; y <= y2; y++) {
        for (x = x1; x <= x2; x++) {
            active_gui->set_pixel(x, y, color);
        }
    }
}

void UG_DrawHLine(UG_S16 x1, UG_S16 x2, UG_S16 y, UG_COLOR color)
{
    UG_S16 x;

    if (active_gui == 0 || active_gui->set_pixel == 0) {
        return;
    }
    if (x1 > x2) {
        ug_swap_s16(&x1, &x2);
    }
    if (y < 0 || y >= active_gui->y_dim || x2 < 0 || x1 >= active_gui->x_dim) {
        return;
    }
    if (x1 < 0) {
        x1 = 0;
    }
    if (x2 >= active_gui->x_dim) {
        x2 = active_gui->x_dim - 1;
    }
    for (x = x1; x <= x2; x++) {
        active_gui->set_pixel(x, y, color);
    }
}

void UG_DrawVLine(UG_S16 x, UG_S16 y1, UG_S16 y2, UG_COLOR color)
{
    UG_S16 y;

    if (active_gui == 0 || active_gui->set_pixel == 0) {
        return;
    }
    if (y1 > y2) {
        ug_swap_s16(&y1, &y2);
    }
    if (x < 0 || x >= active_gui->x_dim || y2 < 0 || y1 >= active_gui->y_dim) {
        return;
    }
    if (y1 < 0) {
        y1 = 0;
    }
    if (y2 >= active_gui->y_dim) {
        y2 = active_gui->y_dim - 1;
    }
    for (y = y1; y <= y2; y++) {
        active_gui->set_pixel(x, y, color);
    }
}

void UG_DrawFrame(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR color)
{
    if (ug_clip_frame(&x1, &y1, &x2, &y2) == UG_FALSE) {
        return;
    }
    UG_DrawHLine(x1, x2, y1, color);
    UG_DrawHLine(x1, x2, y2, color);
    UG_DrawVLine(x1, y1, y2, color);
    UG_DrawVLine(x2, y1, y2, color);
}

void UG_DrawLine(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR color)
{
    int16_t dx;
    int16_t sx;
    int16_t dy;
    int16_t sy;
    int16_t err;
    int16_t e2;

    if (active_gui == 0 || active_gui->set_pixel == 0) {
        return;
    }

    dx = (x1 < x2) ? (x2 - x1) : (x1 - x2);
    sx = (x1 < x2) ? 1 : -1;
    dy = (y1 < y2) ? (y1 - y2) : (y2 - y1);
    sy = (y1 < y2) ? 1 : -1;
    err = dx + dy;

    while (1) {
        UG_DrawPixel(x1, y1, color);
        if (x1 == x2 && y1 == y2) {
            break;
        }
        e2 = (int16_t)(2 * err);
        if (e2 >= dy) {
            err = (int16_t)(err + dy);
            x1 = (UG_S16)(x1 + sx);
        }
        if (e2 <= dx) {
            err = (int16_t)(err + dx);
            y1 = (UG_S16)(y1 + sy);
        }
    }
}

void UG_PutStringColor(UG_S16 x, UG_S16 y, const char *str, UG_COLOR fg, UG_COLOR bg)
{
    if (active_gui == 0 || active_gui->put_string == 0 || str == 0) {
        return;
    }
    active_gui->put_string(x, y, str, fg, bg);
}

void UG_PutString(UG_S16 x, UG_S16 y, const char *str)
{
    UG_PutStringColor(x, y, str, UG_GetForecolor(), UG_GetBackcolor());
}

static UG_S16 ug_text_x_center(UG_S16 x1, UG_S16 x2, const char *text)
{
    UG_S16 len = 0;
    UG_S16 width;

    if (text != 0) {
        len = (UG_S16)strlen(text);
    }
    width = (UG_S16)(len * 8);
    if ((x2 - x1 + 1) <= width) {
        return (UG_S16)(x1 + 2);
    }
    return (UG_S16)(x1 + (((x2 - x1 + 1) - width) / 2));
}

void UG_DrawButton(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2,
                   const char *text, UG_BOOL selected, UG_BOOL active)
{
    UG_COLOR bg = active ? UG_COLOR_ACCENT : UG_COLOR_CARD;
    UG_COLOR edge = selected ? UG_COLOR_WARN : UG_COLOR_CARD2;
    UG_COLOR text_color = active ? UG_COLOR_WHITE : UG_COLOR_TEXT;
    UG_S16 tx;
    UG_S16 ty;

    UG_FillFrame(x1, y1, x2, y2, bg);
    UG_DrawFrame(x1, y1, x2, y2, edge);
    if (selected != UG_FALSE) {
        UG_DrawFrame((UG_S16)(x1 + 1), (UG_S16)(y1 + 1), (UG_S16)(x2 - 1), (UG_S16)(y2 - 1), edge);
    }
    tx = ug_text_x_center(x1, x2, text);
    ty = (UG_S16)(y1 + (((y2 - y1 + 1) - 16) / 2));
    if (ty < y1 + 1) {
        ty = (UG_S16)(y1 + 1);
    }
    UG_PutStringColor(tx, ty, text, text_color, bg);
}

void UG_DrawToggle(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2,
                   const char *label, UG_BOOL selected, UG_BOOL on)
{
    UG_COLOR bg = selected ? UG_COLOR_CARD2 : UG_COLOR_CARD;
    UG_COLOR edge = selected ? UG_COLOR_WARN : UG_COLOR_DARK_GRAY;
    UG_COLOR knob = on ? UG_COLOR_OK : UG_COLOR_MUTED;
    UG_COLOR track = on ? UG_RGB565(13, 87, 63) : UG_RGB565(55, 65, 81);
    UG_S16 sw_x1 = (UG_S16)(x2 - 39);
    UG_S16 sw_y1 = (UG_S16)(y1 + 5);
    UG_S16 sw_x2 = (UG_S16)(x2 - 6);
    UG_S16 sw_y2 = (UG_S16)(y2 - 5);
    UG_S16 knob_x1;
    UG_S16 knob_x2;

    UG_FillFrame(x1, y1, x2, y2, bg);
    UG_DrawFrame(x1, y1, x2, y2, edge);
    if (selected != UG_FALSE) {
        UG_DrawFrame((UG_S16)(x1 + 1), (UG_S16)(y1 + 1), (UG_S16)(x2 - 1), (UG_S16)(y2 - 1), edge);
    }
    UG_PutStringColor((UG_S16)(x1 + 6), (UG_S16)(y1 + 4), label, UG_COLOR_TEXT, bg);

    UG_FillFrame(sw_x1, sw_y1, sw_x2, sw_y2, track);
    UG_DrawFrame(sw_x1, sw_y1, sw_x2, sw_y2, UG_COLOR_LIGHT_GRAY);
    if (on != UG_FALSE) {
        knob_x1 = (UG_S16)(sw_x2 - 12);
        knob_x2 = (UG_S16)(sw_x2 - 2);
    } else {
        knob_x1 = (UG_S16)(sw_x1 + 2);
        knob_x2 = (UG_S16)(sw_x1 + 12);
    }
    UG_FillFrame(knob_x1, (UG_S16)(sw_y1 + 2), knob_x2, (UG_S16)(sw_y2 - 2), knob);
}

void UG_DrawProgress(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2,
                     UG_U8 percent, UG_COLOR fill_color)
{
    UG_S16 inner_x1;
    UG_S16 inner_y1;
    UG_S16 inner_x2;
    UG_S16 inner_y2;
    UG_S16 fill_x2;
    UG_U16 width;

    if (percent > 100u) {
        percent = 100u;
    }

    UG_FillFrame(x1, y1, x2, y2, UG_COLOR_CARD);
    UG_DrawFrame(x1, y1, x2, y2, UG_COLOR_CARD2);
    inner_x1 = (UG_S16)(x1 + 2);
    inner_y1 = (UG_S16)(y1 + 2);
    inner_x2 = (UG_S16)(x2 - 2);
    inner_y2 = (UG_S16)(y2 - 2);
    UG_FillFrame(inner_x1, inner_y1, inner_x2, inner_y2, UG_RGB565(15, 23, 42));
    width = (UG_U16)(inner_x2 - inner_x1 + 1);
    fill_x2 = (UG_S16)(inner_x1 + (((uint32_t)width * percent) / 100u) - 1);
    if (percent > 0u) {
        UG_FillFrame(inner_x1, inner_y1, fill_x2, inner_y2, fill_color);
    }
}
