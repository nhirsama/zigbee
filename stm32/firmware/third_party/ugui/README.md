# UGUI subset

This folder contains a small UGUI-compatible drawing subset used by the STM32F103RC + ST7735S firmware.
It keeps the API intentionally compact for the 128x160 screen and 48 KiB SRAM target:

- screen/frame/line drawing
- LCD text callback hook
- button/toggle/progress helper widgets

The LCD binding lives in `firmware/drivers/display/ugui_port.c`.
