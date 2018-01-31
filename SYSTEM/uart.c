/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : uast.c
*Auther         : 张沁
*Version        : v2.0  兼容uCOS-II
*Date           : 2017-04-15
*Description    : 该文件包含了有关uart初始化配置以及中断使能的函数，
将接收到的数据保存到一个数组，使得串口接收的最大字节数为2的14次方减1，
同时通信协议里面要求传输的数据以回车换行符（\r\n结束）
*Function List  : 
*Histoty        :
1. Date: 2017-04-15    Auther: 张沁
   Modifycation: 添加uCOS-II兼容程序
*************************************************************/
#include "myinclude.h"

#if SYSTEM_SUPPORT_OS
#include "includes.h"  //ucos 使用	  
#endif

//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
	USART1->DR = (u8) ch;      
	return ch;
}
#endif

u8 UART_RX_BUF[UART_REC_LEN];  //接收缓冲

/*
接收状态
bit15 接收完成
bit14 接收到0x0d '\r'
bit13~0 接收到的有效字符数目
*/
u16 USART_RX_STA = 0;  //接收状态标记为0

/*************************************************************
*Function Name  : UART1_Init
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2016-08-25 
*Description    : 用于uart1的初始化
*Input          ：1. bound  串口通信的波特率
*Output         ：
*Return         ：
*************************************************************/
void UART1_Init(u32 bound)
{
	GPIO_InitTypeDef GPIOA_InitStructure;
	USART_InitTypeDef UART1_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/*******初始化GPIOA和USART1的时钟************/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	/*****复用GPIOA9和GPIO10为USART1***********/
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
	
	GPIOA_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIOA_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIOA_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	GPIOA_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIOA_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	
    GPIO_Init(GPIOA, &GPIOA_InitStructure);
	
	UART1_InitStructure.USART_BaudRate = bound;  //波特率
	UART1_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  //无硬件流控制
	UART1_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;  //接收和发送模式
	UART1_InitStructure.USART_Parity = USART_Parity_No;  //无奇偶校验
	UART1_InitStructure.USART_StopBits = USART_StopBits_1;  //1个停止位
	UART1_InitStructure.USART_WordLength = USART_WordLength_8b;  //8个字长
	
	USART_Init(USART1, &UART1_InitStructure);  //UART1初始化
	
	/*******UART1的接收中断配置************/
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART1, ENABLE);  //使能UART1
}

/*************************************************************
*Function Name  : USART1_IRQHandler
*Auther         : 张沁
*Vertion        : v2.0
*Date           : 2017-04-15
*Description    : USART1的中断服务例程，包含了传输协议（以回车换行符结束）
*Input          ：
*Output         ：
*Return         ：
*Histoty        :
1. Date: 2017-04-15    Auther: 张沁
   Modifycation: 添加uCOS-II兼容程序
*************************************************************/
void USART1_IRQHandler(void)
{
	u8 res;  //用于接收数据
	
#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif

	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //如果接收寄存器不为空
	{
		res = USART_ReceiveData(USART1);  //接收数据
		
		if((USART_RX_STA & 0x8000) == 0)  //接收未完成
		{
			/****如果已接收到0x0d，下一步就判断本次就收的是否是0x0a******/            //  ('\r' 0x0d)  ('\n' 0x0a)
			if(USART_RX_STA & 0x4000)  //接收到0x0d
			{
				if(res != 0x0a)  //如果不是0x0a，表示接收错误，重新开始
				{
					USART_RX_STA = 0;
				}
				else  //如果是0x0a，接收完成
				{
					USART_RX_STA |= 0x8000;
				}
			}
			
			/***如果没就收到0x0d，下一步判断接收的是否是0x0d***/
			else
			{
				if(res == 0x0d)
				{
					USART_RX_STA |= 0x4000;
				}
				else
				{
					UART_RX_BUF[USART_RX_STA & 0x3FFF] = res; //接收存放于缓冲区
					USART_RX_STA++;
					
					if((USART_RX_STA & 0x3FFF) > (UART_REC_LEN - 1))  //接收数据超过缓冲区最大容量，接收错误  
					{
						USART_RX_STA = 0;
					}
				}
			}
		}
		
	}
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif
}

/************************************************************/
//Function Name: Clear_Serial
//Auther: 张沁
//Vertion: v1.0
//Date: 2016-09-01
//Description: 字符串清空
//Input：1.USART_RX_BUF[] 要清空的字符串
//Output：
//Return：
/************************************************************/
void Clear_Serial(u8 USART_RX_BUF[])
{	
	u8 index = 0;
	
	while (USART_RX_BUF[index] != '\0')
	{
		USART_RX_BUF[index] = 0;
		index++;
	}
}

