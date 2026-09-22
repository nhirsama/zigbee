#ifndef UGUI_PORT_H_
#define UGUI_PORT_H_

#include "ugui.h"

#ifdef __cplusplus
extern "C" {
#endif

void UGUI_Port_Init(void);
UG_GUI *UGUI_Port_GetGUI(void);

#ifdef __cplusplus
}
#endif

#endif /* UGUI_PORT_H_ */
