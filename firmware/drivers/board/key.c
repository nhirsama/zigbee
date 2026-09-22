#include "key.h"
#include "delay.h"

//按键的初始化函数
void KEY_Config(void)
{
	GPIO_InitTypeDef KEY = {0};
	//开时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB,ENABLE);
	//配置并初始化PA0
	KEY.GPIO_Mode = GPIO_Mode_IN_FLOATING;//模式配置为浮空输入
	KEY.GPIO_Pin = GPIO_Pin_0;//选择管脚
	GPIO_Init(GPIOA,&KEY);
	//配置并初始化PB8
	KEY.GPIO_Pin = GPIO_Pin_8;//选择管脚
	GPIO_Init(GPIOB,&KEY);
}

//按键扫描函数
u8 KEY_Sanf(void)
{
	u8 key_flag = 0;
	//KEY1的检测
	if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 1)
	{
		//延时消抖
		Delay_ms(10);
		if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 1)
		{
			//松手检测
			while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 1)
			{}
			key_flag = 1;
		}
	}
	
	//KEY2的检测
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_8) == 0)
	{
		//延时消抖
		Delay_ms(10);
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_8) == 0)
		{
			//松手检测
			while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_8) == 0)
			{}
			key_flag = 2;
		}
	}
	
	return key_flag;
}