#include "delay.h"

u32 led_runtime = 0;
u32 bafa_runtime = 0;
volatile u32 system_runtime = 0;

//系统定时器的中断服务函数
//1ms自动执行一次
void SysTick_Handler(void)
{
	led_runtime++;//1ms自动向上计数一次
	bafa_runtime++;
	system_runtime++;
}

/*
函数功能：微秒级的延时
函数参数：要延时多少微秒 
*/
void Delay_us(uint32_t time)
{
	while(time--)
		delay_1us();
}

/*
函数功能：毫秒级的延时
函数参数：要延时多少毫秒
*/
void Delay_ms(uint32_t time)
{
	uint64_t ms = time*1000;
	while(ms--)
	{
		delay_1us();
	}
}
uint32_t Delay_GetMillis(void)
{
	return system_runtime;
}
