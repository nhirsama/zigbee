#ifndef _BEEP_H_
#define _BEEP_H_

#include "stm32f10x.h"

#define BEEP_ON GPIO_WriteBit(GPIOA,GPIO_Pin_6,Bit_SET)
#define BEEP_OFF GPIO_WriteBit(GPIOA,GPIO_Pin_6,Bit_RESET)


void BEEP_Config(void);

#endif