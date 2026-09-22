#include "beep.h"

//蜂鸣器的初始化函数
void BEEP_Config(void)
{
	GPIO_InitTypeDef BEEP = {0};
	//开时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	//配置GPIO口
	BEEP.GPIO_Mode = GPIO_Mode_Out_PP;//模式配置为推挽输出
	BEEP.GPIO_Pin = GPIO_Pin_6;//选择管脚
	BEEP.GPIO_Speed = GPIO_Speed_50MHz;//GPIO口的输出效率
	//初始化GPIO口
	GPIO_Init(GPIOA,&BEEP);
	//关闭三盏灯
	BEEP_OFF;
}