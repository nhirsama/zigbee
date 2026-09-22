#include "app_ui.h"

#include <stdio.h>
#include <string.h>

#include "beep.h"
#include "delay.h"
#include "led.h"
#include "spi.h"
#include "ugui.h"
#include "ugui_port.h"
#include "apple_logo_64x78.h"

#define APP_UI_DOUBLE_CLICK_MS 360u
#define APP_UI_VISIBLE_ROWS 5u
#define APP_UI_ROW_H 23
#define APP_UI_ROW_Y0 24
#define APP_UI_CONTENT_Y0 21
#define APP_UI_CONTENT_Y1 143
#define APP_UI_NFC_BEEP_ON_MS 70u
#define APP_UI_NFC_BEEP_OFF_MS 70u

typedef enum {
    APP_MENU_ROOT = 0,
    APP_MENU_QUICK,
    APP_MENU_SENSOR,
    APP_MENU_NETWORK,
    APP_MENU_SYSTEM,
    APP_MENU_ABOUT,
    APP_MENU_COUNT
} app_menu_id_t;

typedef enum {
    APP_ITEM_SUBMENU = 0,
    APP_ITEM_TOGGLE,
    APP_ITEM_VALUE,
    APP_ITEM_ACTION
} app_item_type_t;

typedef enum {
    APP_ACTION_NONE = 0,
    APP_ACTION_LED2,
    APP_ACTION_LED3,
    APP_ACTION_BEEP,
    APP_ACTION_MQTT,
    APP_ACTION_LED1_BLINK,
    APP_ACTION_LCD_BL,
    APP_ACTION_BEEP_PULSE,
    APP_ACTION_ALL_LOCAL_OFF
} app_item_action_t;

typedef struct {
    const char *label;
    app_item_type_t type;
    app_menu_id_t target;
    app_item_action_t action;
} app_menu_item_t;

typedef struct {
    const char *title;
    const app_menu_item_t *items;
    uint8_t count;
} app_menu_t;

typedef struct {
    app_menu_id_t current_menu;
    app_menu_id_t stack[4];
    uint8_t stack_depth;
    uint8_t selected[APP_MENU_COUNT];
    uint8_t top[APP_MENU_COUNT];

    uint8_t led1_blink_on;
    uint8_t led2_on;
    uint8_t led3_on;
    uint8_t beep_on;
    uint8_t mqtt_on;
    uint8_t lcd_bl_on;
    uint8_t network_ok;
    uint8_t sensor_valid;
    uint8_t temperature;
    uint8_t humidity;
    uint8_t mode;
    uint32_t publish_count;
    uint32_t last_publish_a;
    uint32_t last_publish_b;

    uint8_t pending_key;
    uint32_t pending_time;
    uint8_t control_visible;
    uint8_t beep_pulse_active;
    uint8_t beep_pulse_phase;
    uint32_t beep_pulse_next_ms;
} app_ui_state_t;

static const app_menu_item_t root_items[] = {
    {"Quick Ctrl", APP_ITEM_SUBMENU, APP_MENU_QUICK, APP_ACTION_NONE},
    {"Sensors", APP_ITEM_SUBMENU, APP_MENU_SENSOR, APP_ACTION_NONE},
    {"Network", APP_ITEM_SUBMENU, APP_MENU_NETWORK, APP_ACTION_NONE},
    {"System", APP_ITEM_SUBMENU, APP_MENU_SYSTEM, APP_ACTION_NONE},
};

static const app_menu_item_t quick_items[] = {
    {"LED2", APP_ITEM_TOGGLE, APP_MENU_QUICK, APP_ACTION_LED2},
    {"LED3", APP_ITEM_TOGGLE, APP_MENU_QUICK, APP_ACTION_LED3},
    {"BEEP", APP_ITEM_TOGGLE, APP_MENU_QUICK, APP_ACTION_BEEP},
    {"MQTT Pub", APP_ITEM_TOGGLE, APP_MENU_QUICK, APP_ACTION_MQTT},
};

static const app_menu_item_t sensor_items[] = {
    {"NFC", APP_ITEM_VALUE, APP_MENU_SENSOR, APP_ACTION_NONE},
    {"P1", APP_ITEM_VALUE, APP_MENU_SENSOR, APP_ACTION_NONE},
    {"P2", APP_ITEM_VALUE, APP_MENU_SENSOR, APP_ACTION_NONE},
};

static const app_menu_item_t network_items[] = {
    {"MQTT Pub", APP_ITEM_TOGGLE, APP_MENU_NETWORK, APP_ACTION_MQTT},
    {"Net State", APP_ITEM_VALUE, APP_MENU_NETWORK, APP_ACTION_NONE},
    {"Pub Count", APP_ITEM_VALUE, APP_MENU_NETWORK, APP_ACTION_NONE},
    {"Last A", APP_ITEM_VALUE, APP_MENU_NETWORK, APP_ACTION_NONE},
    {"Last B", APP_ITEM_VALUE, APP_MENU_NETWORK, APP_ACTION_NONE},
};

static const app_menu_item_t system_items[] = {
    {"LED1 Blink", APP_ITEM_TOGGLE, APP_MENU_SYSTEM, APP_ACTION_LED1_BLINK},
    {"LCD Light", APP_ITEM_TOGGLE, APP_MENU_SYSTEM, APP_ACTION_LCD_BL},
    {"Beep Pulse", APP_ITEM_ACTION, APP_MENU_SYSTEM, APP_ACTION_BEEP_PULSE},
    {"All Off", APP_ITEM_ACTION, APP_MENU_SYSTEM, APP_ACTION_ALL_LOCAL_OFF},
    {"About", APP_ITEM_SUBMENU, APP_MENU_ABOUT, APP_ACTION_NONE},
};

static const app_menu_item_t about_items[] = {
    {"FW", APP_ITEM_VALUE, APP_MENU_ABOUT, APP_ACTION_NONE},
    {"MCU", APP_ITEM_VALUE, APP_MENU_ABOUT, APP_ACTION_NONE},
    {"LCD", APP_ITEM_VALUE, APP_MENU_ABOUT, APP_ACTION_NONE},
    {"Keys", APP_ITEM_VALUE, APP_MENU_ABOUT, APP_ACTION_NONE},
};

static const app_menu_t menus[APP_MENU_COUNT] = {
    {"Main", root_items, (uint8_t)(sizeof(root_items) / sizeof(root_items[0]))},
    {"Quick", quick_items, (uint8_t)(sizeof(quick_items) / sizeof(quick_items[0]))},
    {"Sensors", sensor_items, (uint8_t)(sizeof(sensor_items) / sizeof(sensor_items[0]))},
    {"Network", network_items, (uint8_t)(sizeof(network_items) / sizeof(network_items[0]))},
    {"System", system_items, (uint8_t)(sizeof(system_items) / sizeof(system_items[0]))},
    {"About", about_items, (uint8_t)(sizeof(about_items) / sizeof(about_items[0]))},
};

static app_ui_state_t ui_state;

static void AppUI_DrawPage(void);
static void AppUI_DrawRowsAll(void);
static void AppUI_DrawSensorNfcLogo(void);

static UG_COLOR AppUI_HeaderColor(void)
{
    return UG_RGB565(15, 23, 42);
}

static const app_menu_t *AppUI_CurrentMenu(void)
{
    return &menus[ui_state.current_menu];
}

static uint8_t AppUI_IsNfcContact(void)
{
    return (ui_state.sensor_valid != 0u &&
            ui_state.temperature == 1u &&
            ui_state.humidity == 1u) ? 1u : 0u;
}

static void AppUI_ApplyOutputs(void)
{
    if (ui_state.led2_on != 0u) {
        LED2_ON;
    } else {
        LED2_OFF;
    }

    if (ui_state.led3_on != 0u) {
        LED3_ON;
    } else {
        LED3_OFF;
    }

    if (ui_state.beep_pulse_active == 0u) {
        if (ui_state.beep_on != 0u) {
            BEEP_ON;
        } else {
            BEEP_OFF;
        }
    }

    if (ui_state.lcd_bl_on != 0u) {
        LCD_BL_CMD(1);
    } else {
        LCD_BL_CMD(0);
    }

    if (ui_state.led1_blink_on == 0u) {
        LED1_OFF;
    }
}

static void AppUI_ServiceBeepPulse(uint32_t now_ms)
{
    if (ui_state.beep_pulse_active == 0u) {
        return;
    }

    if (ui_state.beep_pulse_phase != 0u &&
        (int32_t)(now_ms - ui_state.beep_pulse_next_ms) < 0) {
        return;
    }

    switch (ui_state.beep_pulse_phase) {
        case 0u:
            BEEP_ON;
            ui_state.beep_pulse_next_ms = now_ms + APP_UI_NFC_BEEP_ON_MS;
            ui_state.beep_pulse_phase = 1u;
            break;
        case 1u:
            BEEP_OFF;
            ui_state.beep_pulse_next_ms = now_ms + APP_UI_NFC_BEEP_OFF_MS;
            ui_state.beep_pulse_phase = 2u;
            break;
        case 2u:
            BEEP_ON;
            ui_state.beep_pulse_next_ms = now_ms + APP_UI_NFC_BEEP_ON_MS;
            ui_state.beep_pulse_phase = 3u;
            break;
        default:
            ui_state.beep_pulse_active = 0u;
            ui_state.beep_pulse_phase = 0u;
            AppUI_ApplyOutputs();
            break;
    }
}

static uint8_t AppUI_GetToggle(app_item_action_t action)
{
    switch (action) {
        case APP_ACTION_LED2:
            return ui_state.led2_on;
        case APP_ACTION_LED3:
            return ui_state.led3_on;
        case APP_ACTION_BEEP:
            return ui_state.beep_on;
        case APP_ACTION_MQTT:
            return ui_state.mqtt_on;
        case APP_ACTION_LED1_BLINK:
            return ui_state.led1_blink_on;
        case APP_ACTION_LCD_BL:
            return ui_state.lcd_bl_on;
        default:
            return 0u;
    }
}

static void AppUI_SetToggle(app_item_action_t action, uint8_t value)
{
    value = (value != 0u) ? 1u : 0u;
    switch (action) {
        case APP_ACTION_LED2:
            ui_state.led2_on = value;
            break;
        case APP_ACTION_LED3:
            ui_state.led3_on = value;
            break;
        case APP_ACTION_BEEP:
            ui_state.beep_on = value;
            break;
        case APP_ACTION_MQTT:
            ui_state.mqtt_on = value;
            break;
        case APP_ACTION_LED1_BLINK:
            ui_state.led1_blink_on = value;
            break;
        case APP_ACTION_LCD_BL:
            ui_state.lcd_bl_on = value;
            break;
        default:
            break;
    }
    AppUI_ApplyOutputs();
}

static void AppUI_RunAction(app_item_action_t action)
{
    uint8_t saved_beep;

    switch (action) {
        case APP_ACTION_BEEP_PULSE:
            saved_beep = ui_state.beep_on;
            BEEP_ON;
            Delay_ms(80);
            if (saved_beep != 0u) {
                BEEP_ON;
            } else {
                BEEP_OFF;
            }
            break;
        case APP_ACTION_ALL_LOCAL_OFF:
            ui_state.led2_on = 0u;
            ui_state.led3_on = 0u;
            ui_state.beep_on = 0u;
            AppUI_ApplyOutputs();
            break;
        default:
            break;
    }
}

static void AppUI_FormatValue(const app_menu_item_t *item, char *buf, uint8_t len)
{
    if (buf == 0 || len == 0u) {
        return;
    }

    buf[0] = '\0';

    if (item->type == APP_ITEM_SUBMENU) {
        snprintf(buf, len, ">");
        return;
    }

    if (item->type == APP_ITEM_TOGGLE) {
        snprintf(buf, len, "%s", (AppUI_GetToggle(item->action) != 0u) ? "ON" : "OFF");
        return;
    }

    if (item->type == APP_ITEM_ACTION) {
        snprintf(buf, len, "RUN");
        return;
    }

    if (ui_state.current_menu == APP_MENU_SENSOR) {
        if (strcmp(item->label, "NFC") == 0) {
            if (ui_state.sensor_valid != 0u) {
                snprintf(buf, len, "%s", (AppUI_IsNfcContact() != 0u) ? "TOUCH" : "IDLE");
            } else {
                snprintf(buf, len, "--");
            }
        } else if (strcmp(item->label, "P1") == 0) {
            if (ui_state.sensor_valid != 0u) {
                snprintf(buf, len, "%u", (unsigned int)ui_state.temperature);
            } else {
                snprintf(buf, len, "--");
            }
        } else if (strcmp(item->label, "P2") == 0) {
            if (ui_state.sensor_valid != 0u) {
                snprintf(buf, len, "%u", (unsigned int)ui_state.humidity);
            } else {
                snprintf(buf, len, "--");
            }
        }
    } else if (ui_state.current_menu == APP_MENU_NETWORK) {
        if (strcmp(item->label, "Net State") == 0) {
            snprintf(buf, len, "%s", (ui_state.network_ok != 0u) ? "OK" : "OFF");
        } else if (strcmp(item->label, "Pub Count") == 0) {
            snprintf(buf, len, "%lu", (unsigned long)ui_state.publish_count);
        } else if (strcmp(item->label, "Last A") == 0) {
            snprintf(buf, len, "%lu", (unsigned long)ui_state.last_publish_a);
        } else if (strcmp(item->label, "Last B") == 0) {
            snprintf(buf, len, "%lu", (unsigned long)ui_state.last_publish_b);
        }
    } else if (ui_state.current_menu == APP_MENU_ABOUT) {
        if (strcmp(item->label, "FW") == 0) {
            snprintf(buf, len, "UGUI");
        } else if (strcmp(item->label, "MCU") == 0) {
            snprintf(buf, len, "F103RC");
        } else if (strcmp(item->label, "LCD") == 0) {
            snprintf(buf, len, "ST7735");
        } else if (strcmp(item->label, "Keys") == 0) {
            snprintf(buf, len, "2 BTN");
        }
    }
}

static UG_S16 AppUI_RightTextX(const char *text)
{
    UG_S16 len = 0;
    if (text != 0) {
        len = (UG_S16)strlen(text);
    }
    if (len > 6) {
        len = 6;
    }
    return (UG_S16)(124 - (len * 8));
}

static UG_S16 AppUI_RowY(uint8_t slot)
{
    return (UG_S16)(APP_UI_ROW_Y0 + ((UG_S16)slot * APP_UI_ROW_H));
}

static void AppUI_AdjustScroll(void)
{
    const app_menu_t *menu = AppUI_CurrentMenu();
    uint8_t selected = ui_state.selected[ui_state.current_menu];
    uint8_t *top = &ui_state.top[ui_state.current_menu];

    if (menu->count == 0u) {
        *top = 0u;
        return;
    }
    if (selected >= menu->count) {
        selected = (uint8_t)(menu->count - 1u);
        ui_state.selected[ui_state.current_menu] = selected;
    }
    if (selected < *top) {
        *top = selected;
    } else if (selected >= (uint8_t)(*top + APP_UI_VISIBLE_ROWS)) {
        *top = (uint8_t)(selected - APP_UI_VISIBLE_ROWS + 1u);
    }
}

static void AppUI_DrawHeader(void)
{
    char right[8];
    UG_COLOR header = AppUI_HeaderColor();
    UG_COLOR net_color = (ui_state.network_ok != 0u) ? UG_COLOR_OK : UG_COLOR_WARN;
    const char *title = AppUI_CurrentMenu()->title;

    if (ui_state.control_visible == 0u) {
        return;
    }

    UG_FillFrame(0, 0, 127, 20, header);
    UG_PutStringColor(4, 2, title, UG_COLOR_WHITE, header);
    snprintf(right, sizeof(right), "%s", (ui_state.network_ok != 0u) ? "NET" : "OFF");
    UG_PutStringColor(96, 2, right, net_color, header);
}

static void AppUI_ClearRowSlot(uint8_t slot)
{
    UG_S16 y = AppUI_RowY(slot);
    UG_S16 y0 = (UG_S16)(y - 2);
    UG_S16 y1 = (UG_S16)(y + APP_UI_ROW_H - 3);

    if (y0 < APP_UI_CONTENT_Y0) {
        y0 = APP_UI_CONTENT_Y0;
    }
    if (y1 > APP_UI_CONTENT_Y1) {
        y1 = APP_UI_CONTENT_Y1;
    }

    UG_FillFrame(0, y0, 127, y1, UG_COLOR_PANEL);
}

static void AppUI_DrawRowSlot(uint8_t slot)
{
    const app_menu_t *menu = AppUI_CurrentMenu();
    uint8_t idx;
    uint8_t top;
    uint8_t selected;
    UG_S16 y;
    char value[12];
    UG_COLOR bg;
    UG_COLOR fg;
    UG_COLOR edge;
    const app_menu_item_t *item;

    if (ui_state.control_visible == 0u || slot >= APP_UI_VISIBLE_ROWS) {
        return;
    }

    top = ui_state.top[ui_state.current_menu];
    selected = ui_state.selected[ui_state.current_menu];
    idx = (uint8_t)(top + slot);
    y = AppUI_RowY(slot);

    AppUI_ClearRowSlot(slot);
    if (idx >= menu->count) {
        return;
    }

    item = &menu->items[idx];
    bg = (idx == selected) ? UG_COLOR_CARD2 : UG_COLOR_CARD;
    fg = (idx == selected) ? UG_COLOR_WHITE : UG_COLOR_TEXT;
    edge = (idx == selected) ? UG_COLOR_WARN : UG_COLOR_DARK_GRAY;

    UG_FillFrame(4, y, 123, (UG_S16)(y + 19), bg);
    UG_DrawFrame(4, y, 123, (UG_S16)(y + 19), edge);
    UG_PutStringColor(8, (UG_S16)(y + 2), item->label, fg, bg);

    AppUI_FormatValue(item, value, sizeof(value));
    value[6] = '\0';
    UG_PutStringColor(AppUI_RightTextX(value), (UG_S16)(y + 2), value,
                      (item->type == APP_ITEM_TOGGLE && AppUI_GetToggle(item->action) != 0u) ? UG_COLOR_OK : UG_COLOR_MUTED,
                      bg);
}

static void AppUI_DrawRowByIndex(uint8_t item_index)
{
    uint8_t top = ui_state.top[ui_state.current_menu];

    if (item_index < top || item_index >= (uint8_t)(top + APP_UI_VISIBLE_ROWS)) {
        return;
    }

    AppUI_DrawRowSlot((uint8_t)(item_index - top));
}

static void AppUI_DrawRowsAll(void)
{
    uint8_t i;

    if (ui_state.control_visible == 0u) {
        return;
    }

    AppUI_AdjustScroll();

    for (i = 0u; i < APP_UI_VISIBLE_ROWS; i++) {
        AppUI_DrawRowSlot(i);
    }
}

static void AppUI_DrawSensorNfcLogo(void)
{
    uint8_t open = AppUI_IsNfcContact();
    UG_COLOR bg = (open != 0u) ? UG_RGB565(6, 78, 59) : UG_COLOR_CARD;
    UG_COLOR edge = (open != 0u) ? UG_COLOR_OK : UG_COLOR_DARK_GRAY;
    UG_COLOR fg = (open != 0u) ? UG_COLOR_OK : UG_COLOR_MUTED;
    UG_COLOR text = (open != 0u) ? UG_COLOR_WHITE : UG_COLOR_MUTED;

    if (ui_state.control_visible == 0u || ui_state.current_menu != APP_MENU_SENSOR) {
        return;
    }

    UG_FillFrame(4, 92, 123, 141, bg);
    UG_DrawFrame(4, 92, 123, 141, edge);
    UG_DrawFrame(5, 93, 122, 140, (open != 0u) ? UG_RGB565(20, 112, 82) : UG_COLOR_CARD2);

    /* Door frame. */
    UG_DrawFrame(13, 101, 41, 135, fg);
    UG_DrawHLine(17, 37, 105, fg);
    UG_DrawVLine(17, 105, 135, fg);

    if (open != 0u) {
        /* Open door panel, drawn as a tilted outline. */
        UG_DrawLine(41, 101, 58, 108, UG_COLOR_WHITE);
        UG_DrawLine(58, 108, 58, 132, UG_COLOR_WHITE);
        UG_DrawLine(58, 132, 41, 135, UG_COLOR_WHITE);
        UG_DrawLine(41, 101, 41, 135, UG_COLOR_WHITE);
        UG_FillFrame(52, 120, 55, 123, UG_COLOR_WARN);
        UG_PutStringColor(75, 99, "OPEN", UG_COLOR_WHITE, bg);
        UG_PutStringColor(65, 119, "NFC OK", UG_COLOR_OK, bg);
    } else {
        /* Closed door / waiting state. */
        UG_DrawVLine(27, 106, 135, fg);
        UG_FillFrame(31, 119, 34, 122, fg);
        UG_DrawFrame(73, 100, 113, 125, fg);
        UG_PutStringColor(81, 105, "NFC", fg, bg);
        UG_PutStringColor(73, 123, "WAIT", text, bg);
    }

    if (open != 0u) {
        /* NFC waves. */
        UG_DrawLine(68, 111, 72, 106, fg);
        UG_DrawLine(72, 106, 72, 130, fg);
        UG_DrawLine(72, 130, 68, 125, fg);
        UG_DrawLine(62, 111, 66, 103, fg);
        UG_DrawLine(66, 103, 66, 133, fg);
        UG_DrawLine(66, 133, 62, 125, fg);
    }
}

static void AppUI_DrawFooter(void)
{
    UG_COLOR header = AppUI_HeaderColor();

    if (ui_state.control_visible == 0u) {
        return;
    }

    UG_FillFrame(0, 144, 127, 159, header);
    UG_PutStringColor(0, 144, "K1 D/U K2 OK/BK", UG_COLOR_MUTED, header);
}

static void AppUI_DrawPage(void)
{
    if (ui_state.control_visible == 0u) {
        return;
    }

    AppUI_DrawHeader();
    UG_FillFrame(0, APP_UI_CONTENT_Y0, 127, APP_UI_CONTENT_Y1, UG_COLOR_PANEL);
    AppUI_DrawRowsAll();
    AppUI_DrawSensorNfcLogo();
    AppUI_DrawFooter();
}

static void AppUI_DrawBootLogo(UG_S16 x0, UG_S16 y0, UG_COLOR color)
{
    uint8_t x;
    uint8_t y;

    for (y = 0u; y < APPLE_LOGO_64X78_H; y++) {
        for (x = 0u; x < APPLE_LOGO_64X78_W; x++) {
            uint16_t byte_index = (uint16_t)y * APPLE_LOGO_64X78_STRIDE + (uint16_t)(x >> 3);
            uint8_t mask = (uint8_t)(0x80u >> (x & 0x07u));
            if ((apple_logo_64x78_bits[byte_index] & mask) != 0u) {
                UG_DrawPixel((UG_S16)(x0 + x), (UG_S16)(y0 + y), color);
            }
        }
    }
}

void AppUI_BootBegin(void)
{
    ui_state.control_visible = 0u;
    UG_FillScreen(UG_COLOR_BLACK);
    AppUI_DrawBootLogo(32, 18, UG_COLOR_WHITE);
    UG_DrawFrame(17, 123, 110, 130, UG_RGB565(71, 85, 105));
    UG_PutStringColor(28, 140, "BOOT 00%", UG_COLOR_MUTED, UG_COLOR_BLACK);
}

void AppUI_BootProgress(uint8_t done, uint8_t total, const char *label)
{
    uint16_t fill_w;
    uint8_t percent;
    char text[20];

    if (total == 0u) {
        total = 1u;
    }
    if (done > total) {
        done = total;
    }

    fill_w = (uint16_t)(((uint32_t)done * 90u) / total);
    percent = (uint8_t)(((uint32_t)done * 100u) / total);

    UG_FillFrame(18, 124, 109, 129, UG_COLOR_BLACK);
    if (fill_w > 0u) {
        UG_FillFrame(19, 125, (UG_S16)(18u + fill_w), 128, UG_COLOR_WHITE);
    }

    UG_FillFrame(0, 104, 127, 119, UG_COLOR_BLACK);
    if (label != 0) {
        UG_PutStringColor(4, 104, label, UG_COLOR_MUTED, UG_COLOR_BLACK);
    }

    UG_FillFrame(0, 140, 127, 155, UG_COLOR_BLACK);
    snprintf(text, sizeof(text), "BOOT %03u%%", (unsigned int)percent);
    UG_PutStringColor(28, 140, text, UG_COLOR_MUTED, UG_COLOR_BLACK);
}

void AppUI_ShowControlInterface(void)
{
    if (ui_state.control_visible != 0u) {
        return;
    }

    ui_state.control_visible = 1u;
    UG_FillScreen(UG_COLOR_PANEL);
    AppUI_DrawPage();
}

static void AppUI_MoveDown(void)
{
    const app_menu_t *menu = AppUI_CurrentMenu();
    uint8_t *selected = &ui_state.selected[ui_state.current_menu];
    uint8_t old_selected;
    uint8_t old_top;

    if (menu->count == 0u) {
        return;
    }
    old_selected = *selected;
    old_top = ui_state.top[ui_state.current_menu];
    *selected = (uint8_t)((*selected + 1u) % menu->count);
    AppUI_AdjustScroll();
    if (ui_state.top[ui_state.current_menu] != old_top) {
        AppUI_DrawRowsAll();
    } else {
        AppUI_DrawRowByIndex(old_selected);
        AppUI_DrawRowByIndex(*selected);
    }
}

static void AppUI_MoveUp(void)
{
    const app_menu_t *menu = AppUI_CurrentMenu();
    uint8_t *selected = &ui_state.selected[ui_state.current_menu];
    uint8_t old_selected;
    uint8_t old_top;

    if (menu->count == 0u) {
        return;
    }
    old_selected = *selected;
    old_top = ui_state.top[ui_state.current_menu];
    if (*selected == 0u) {
        *selected = (uint8_t)(menu->count - 1u);
    } else {
        *selected = (uint8_t)(*selected - 1u);
    }
    AppUI_AdjustScroll();
    if (ui_state.top[ui_state.current_menu] != old_top) {
        AppUI_DrawRowsAll();
    } else {
        AppUI_DrawRowByIndex(old_selected);
        AppUI_DrawRowByIndex(*selected);
    }
}

static void AppUI_EnterOrToggle(void)
{
    const app_menu_t *menu = AppUI_CurrentMenu();
    const app_menu_item_t *item;
    uint8_t selected;

    if (menu->count == 0u) {
        return;
    }
    selected = ui_state.selected[ui_state.current_menu];
    if (selected >= menu->count) {
        selected = 0u;
    }
    item = &menu->items[selected];

    if (item->type == APP_ITEM_SUBMENU) {
        if (ui_state.stack_depth < (uint8_t)(sizeof(ui_state.stack) / sizeof(ui_state.stack[0]))) {
            ui_state.stack[ui_state.stack_depth++] = ui_state.current_menu;
            ui_state.current_menu = item->target;
            AppUI_DrawPage();
        }
    } else if (item->type == APP_ITEM_TOGGLE) {
        AppUI_SetToggle(item->action, (uint8_t)!AppUI_GetToggle(item->action));
        AppUI_DrawRowByIndex(selected);
    } else if (item->type == APP_ITEM_ACTION) {
        AppUI_RunAction(item->action);
        AppUI_DrawRowByIndex(selected);
    }
}

static void AppUI_Back(void)
{
    if (ui_state.stack_depth > 0u) {
        ui_state.stack_depth--;
        ui_state.current_menu = ui_state.stack[ui_state.stack_depth];
        AppUI_DrawPage();
    }
}

static void AppUI_DispatchClick(uint8_t key_code, uint8_t double_click)
{
    if (key_code == 1u) {
        if (double_click != 0u) {
            AppUI_MoveUp();
        } else {
            AppUI_MoveDown();
        }
    } else if (key_code == 2u) {
        if (double_click != 0u) {
            AppUI_Back();
        } else {
            AppUI_EnterOrToggle();
        }
    }
}

void AppUI_Init(void)
{
    memset(&ui_state, 0, sizeof(ui_state));
    ui_state.current_menu = APP_MENU_ROOT;
    ui_state.led1_blink_on = 1u;
    ui_state.mqtt_on = 1u;
    ui_state.lcd_bl_on = 1u;
    ui_state.control_visible = 0u;

    UGUI_Port_Init();
    AppUI_ApplyOutputs();
}

void AppUI_SetNetworkStatus(uint8_t connected)
{
    uint8_t value = (connected != 0u) ? 1u : 0u;
    if (ui_state.network_ok != value) {
        ui_state.network_ok = value;
        AppUI_DrawHeader();
        if (ui_state.current_menu == APP_MENU_NETWORK) {
            AppUI_DrawRowByIndex(1u);
        }
    }
}

void AppUI_SetSensor(const sensor_node_t *node)
{
    uint8_t old_valid;
    uint8_t old_temperature;
    uint8_t old_humidity;
    uint8_t old_nfc_contact;
    uint8_t new_nfc_contact;

    if (node == 0) {
        return;
    }

    old_valid = ui_state.sensor_valid;
    old_temperature = ui_state.temperature;
    old_humidity = ui_state.humidity;
    old_nfc_contact = AppUI_IsNfcContact();

    ui_state.sensor_valid = 1u;
    ui_state.temperature = node->t_val;
    ui_state.humidity = node->h_val;
    ui_state.mode = node->mode;
    new_nfc_contact = AppUI_IsNfcContact();

    if (ui_state.current_menu == APP_MENU_SENSOR) {
        if (old_valid == 0u || old_nfc_contact != new_nfc_contact) {
            AppUI_DrawRowByIndex(0u);
            AppUI_DrawSensorNfcLogo();
        }
        if (old_valid == 0u || old_temperature != ui_state.temperature) {
            AppUI_DrawRowByIndex(1u);
        }
        if (old_valid == 0u || old_humidity != ui_state.humidity) {
            AppUI_DrawRowByIndex(2u);
        }
    }
}

void AppUI_NotePublish(uint32_t first, uint32_t second)
{
    uint8_t first_changed = (ui_state.last_publish_a != first) ? 1u : 0u;
    uint8_t second_changed = (ui_state.last_publish_b != second) ? 1u : 0u;

    ui_state.publish_count++;
    ui_state.last_publish_a = first;
    ui_state.last_publish_b = second;
    if (ui_state.current_menu == APP_MENU_NETWORK) {
        AppUI_DrawRowByIndex(2u);
        if (first_changed != 0u) {
            AppUI_DrawRowByIndex(3u);
        }
        if (second_changed != 0u) {
            AppUI_DrawRowByIndex(4u);
        }
    }
}

void AppUI_BeepShortTwice(void)
{
    ui_state.beep_pulse_active = 1u;
    ui_state.beep_pulse_phase = 0u;
    ui_state.beep_pulse_next_ms = Delay_GetMillis();
    AppUI_ServiceBeepPulse(ui_state.beep_pulse_next_ms);
}

void AppUI_HandleKey(uint8_t key_code, uint32_t now_ms)
{
    if (key_code == 0u) {
        return;
    }

    if (ui_state.pending_key != 0u) {
        if (ui_state.pending_key == key_code && (uint32_t)(now_ms - ui_state.pending_time) <= APP_UI_DOUBLE_CLICK_MS) {
            ui_state.pending_key = 0u;
            AppUI_DispatchClick(key_code, 1u);
            return;
        }
        AppUI_DispatchClick(ui_state.pending_key, 0u);
        ui_state.pending_key = 0u;
    }

    ui_state.pending_key = key_code;
    ui_state.pending_time = now_ms;
}

void AppUI_Tick(uint32_t now_ms)
{
    AppUI_ServiceBeepPulse(now_ms);

    if (ui_state.pending_key != 0u && (uint32_t)(now_ms - ui_state.pending_time) > APP_UI_DOUBLE_CLICK_MS) {
        uint8_t key = ui_state.pending_key;
        ui_state.pending_key = 0u;
        AppUI_DispatchClick(key, 0u);
    }
}

uint8_t AppUI_IsPublishEnabled(void)
{
    return ui_state.mqtt_on;
}

uint8_t AppUI_IsLed1BlinkEnabled(void)
{
    return ui_state.led1_blink_on;
}
