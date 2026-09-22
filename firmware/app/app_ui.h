#ifndef APP_UI_H_
#define APP_UI_H_

#include <stdint.h>

#include "sensor.h"

#ifdef __cplusplus
extern "C" {
#endif

void AppUI_Init(void);
void AppUI_BootBegin(void);
void AppUI_BootProgress(uint8_t done, uint8_t total, const char *label);
void AppUI_ShowControlInterface(void);
void AppUI_SetNetworkStatus(uint8_t connected);
void AppUI_SetSensor(const sensor_node_t *node);
void AppUI_NotePublish(uint32_t first, uint32_t second);
void AppUI_BeepShortTwice(void);
void AppUI_HandleKey(uint8_t key_code, uint32_t now_ms);
void AppUI_Tick(uint32_t now_ms);
uint8_t AppUI_IsPublishEnabled(void);
uint8_t AppUI_IsLed1BlinkEnabled(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_UI_H_ */
