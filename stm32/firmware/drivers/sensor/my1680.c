#include "my1680.h"

/*
函数功能：my1680模块的初始化函数
PC10--UART4_TX   PC11--UART4_RX
*/
void MY1680_Config(void)
{
	GPIO_InitTypeDef GPIO_UART4 = {0};//用于配置GPIO口的结构体变量 
	USART_InitTypeDef UART_4 = {0};//用于配置串口4的结构体变量
	//开时钟（GPIOC、UART4）
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);
	//配置并初始化PC10(复用推挽)
	GPIO_UART4.GPIO_Mode = GPIO_Mode_AF_PP;//复用推挽
	GPIO_UART4.GPIO_Pin = GPIO_Pin_10;//选择管脚PC10
	GPIO_UART4.GPIO_Speed = GPIO_Speed_50MHz;//GPIO口的输出速率，选最大即可
	GPIO_Init(GPIOC,&GPIO_UART4);
	//配置并初始化PC11（浮空输入）
	GPIO_UART4.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_UART4.GPIO_Pin = GPIO_Pin_11;//选择管脚PC11
	GPIO_Init(GPIOC,&GPIO_UART4);
	//配置串口4
	UART_4.USART_BaudRate = 9600;//波特率
	UART_4.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//不使用硬件流控
	UART_4.USART_Mode = USART_Mode_Tx|USART_Mode_Rx;//接收发送同时打开
	UART_4.USART_Parity = USART_Parity_No;//不使用奇偶校验
	UART_4.USART_StopBits = USART_StopBits_1;//1位停止位
	UART_4.USART_WordLength = USART_WordLength_8b;//8位数据位
	//初始化串口4
	USART_Init(UART4,&UART_4);
	//使能串口4
	USART_Cmd(UART4,ENABLE);
}

/*
函数功能：串口4发送单字节
*/
void UART4_SendData(u8 data)
{
	//等待上一帧数据发送完成
	while(USART_GetFlagStatus(UART4,USART_FLAG_TXE) == RESET);
	//发送新一帧数据
	USART_SendData(UART4,data);
}

/*
函数功能：播放MY1680根目录中的曲目
函数形参：
	H:曲目高位
	L:曲目低位
*/
void Player_41(u8 H,u8 L)
{
	UART4_SendData(0x7E);//起始码
	UART4_SendData(0x05);//长度
	UART4_SendData(0x41);//41操作码表示播放根目录中的曲目
	UART4_SendData(H);//曲目高位
	UART4_SendData(L);//曲目低位
	u8 cheak = 0x05^0x41^H^L;
	UART4_SendData(cheak);
	UART4_SendData(0xEF);//结束码
}

/*
函数功能：播放MY1680文件夹中的曲目
函数形参：
	H:文件夹名
	L:曲目名
*/
void Player_42(u8 H,u8 L)
{
	UART4_SendData(0x7E);//起始码
	UART4_SendData(0x05);//长度
	UART4_SendData(0x42);//42操作码表示播放文件夹中的曲目
	UART4_SendData(H);//文件夹名
	UART4_SendData(L);//曲目名
	u8 cheak = 0x05^0x42^H^L;
	UART4_SendData(cheak);
	UART4_SendData(0xEF);//结束码
}

/*
函数功能：增加my1680的音量
*/
void Player_15()
{
	UART4_SendData(0x7E);//起始码
	UART4_SendData(0x03);//长度
	UART4_SendData(0x15);//操作码
	UART4_SendData(0x16);//校验码
	UART4_SendData(0xEF);//结束码
}

/*
函数功能：减小my1680的音量
*/
void Player_16()
{
	UART4_SendData(0x7E);//起始码
	UART4_SendData(0x03);//长度
	UART4_SendData(0x16);//操作码
	UART4_SendData(0x15);//校验码
	UART4_SendData(0xEF);//结束码
}

/*
函数功能：播报绝对值小于1000的整数
函数参数：要播报的数字
*/
void Player_num(int num)
{
	u8 i,j,k;//分别用来保存num的百位十位和个位
	if(num < 0)
	{
		Player_42(00,24);//播报负号
		num = -num;
	}
	if(num<20)
	{
		Player_42(00,num);
		return;
	}
	i = num/100;//num百位的计算
	j = (num%100)/10;//num十位的计算
	k = num%10;//num个位的计算
	//如果有百位
	if(i != 0)
	{
		Player_42(00,i);//播报百位的数值
		Player_42(00,20);//播报单位百
	}
	//如果有十位
	if(j != 0)
	{
		Player_42(00,j);//播报十位的数值
		Player_42(00,10);//播报单位十
	}
	//如果有个位
	if(k != 0)
	{
		//十位为0先播0
		if(j == 0)
		{
			Player_42(00,0);//播0
		}
		Player_42(00,k);//播报个位的数值
	}
}

