/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : esp8266_uart.c
*Auther         : 张沁
*Version        : V1.0
*Date           : 2017-04-19
*Description    : 该文件对按键及按键中断进行了初始化
*Function List  : 
*Histoty		:
1. Date: 2017-04-15    Auther: 张沁
   Modifycation: 添加uCOS-II兼容程序
*************************************************************/

#include "myinclude.h"

#if SYSTEM_SUPPORT_OS
#include "includes.h"  //ucos 使用	  
#endif

extern WIFI_Data  Wifi_Data_Buf;
extern u8 startFlag;
extern OS_EVENT *semWifi;
extern OS_EVENT *semWifiData;

/*************************************************************
*Function Name  : UART3_Init
*Auther         : 张沁
*Version        : V2.0
*Date           : 2017-04-16
*Description    : 用于uart3的初始化
*Input          ：1. bound  串口通信的波特率
*Output         ：
*Return         ：
*************************************************************/
void UART3_Init(u32 bound)
{
	GPIO_InitTypeDef GPIOA_InitStructure;
	USART_InitTypeDef UART3_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/*******初始化GPIOB和USART3的时钟************/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);  
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	
	/*****复用GPIOB10和GPIOB11为USART3***********/
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART1);       //PB10:Tx
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART1);       //PB11:Rx
	
	GPIOA_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIOA_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIOA_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIOA_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIOA_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	
    GPIO_Init(GPIOB, &GPIOA_InitStructure);
	
	UART3_InitStructure.USART_BaudRate = bound;                      //波特率
	UART3_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  //无硬件流控制
	UART3_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;  //接收和发送模式
	UART3_InitStructure.USART_Parity = USART_Parity_No;              //无奇偶校验
	UART3_InitStructure.USART_StopBits = USART_StopBits_1;           //1个停止位
	UART3_InitStructure.USART_WordLength = USART_WordLength_8b;      //8个字长
	
	USART_Init(USART3, &UART3_InitStructure);                        //UART1初始化
	
	/*******UART3的接收中断配置************/
	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);
	/******* 使能串口3总线空闲中断 *******/
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
	
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART3, ENABLE);  //使能UART3
	
	USART_ClearITPendingBit(USART3, USART_IT_RXNE);
	USART_ClearITPendingBit(USART3, USART_IT_IDLE);
}

#if 0
/*************************************************************
*Function Name  : itoa
*Auther         : 张沁
*Version        : V1.0
*Date           : 2017-04-19
*Description    : 将整形数据转换成字符串
*Input          ：1. value   要转换的整形数
                  2. *string 转换后的字符串指针
                  3. radix   表示10进制，其他结果为0
*Output         ：转换字符串存储区的首地址
*Return         ：
*************************************************************/
static char *itoa( int value, char *string, int radix )
{
	int     i, d;
    int     flag = 0;
    char    *ptr = string;
	
	/* This implementation only works for decimal numbers. */
	if (radix != 10)
	{
		*ptr = 0;
		return string;
	}
	
	if (!value)
	{
		*ptr++ = 0x30;
		*ptr = 0;
		return string;
	}
	
	/* if this is a negative value insert the minus sign. */
	if (value < 0)
	{
		*ptr++ = '-';
		
		/* Make the value positive. */
        value *= -1;
	}
	
	for (i=10000; i>0; i/=10)
	{
		d = value / i;
		
		if (d || flag)
		{
			*(ptr++) = (char)(d + 0x30);  //0x30 = '0'
			value -= (d * i);
			flag = 1;
		}
	}
	
	*ptr = 0;
	
	return string;
}
#endif

/*************************************************************
*Function Name  : USART3_printf
*Auther         : 张沁
*Version        : V1.0
*Date           : 2017-04-19
*Description    : 格式化输出，类似于C库中的printf，但这里没有用到C库
                  能够完成 "\r" "\n"   "%d"  "%s" 的解析
*Input          ：1. *USARTx 串口通道
                  2. *Data   要发送到串口的内容的指针
                  3. ...     其他参数
*Output         ：
*Return         ：
*************************************************************/
void USART3_printf( USART_TypeDef* USARTx, char *Data, ... )
{
#if 0
	const char *s;
	int d;
#endif
	
	char buf[128];
	char *ptr =buf;

	va_list ap;
	va_start(ap, Data);
	
#if 1
	vsprintf(buf, Data, ap);
	va_end(ap);
	
	while (*ptr != '\0')
	{
		USART_SendData(USARTx, *ptr++);
		while( USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET );
	}
#endif
	
#if 0
	while (*Data != 0)  //判断是否达到字符串结束符
	{
		if (*Data == 0x5c)                          // '\'
		{
			switch (*++Data)
			{
				case 'r':                              
					USART_SendData(USARTx, 0x0d);   //回车符
				    Data++;
				    break;
				
				case 'n':                           //换行符
					USART_SendData(USARTx, 0x0a);
				    Data++;
				    break;
				
				default:
					Data++;
				    break;
			}
		}
		
		else if (*Data == '%')
		{
			switch (*++Data)
			{
				case 's':                           //字符串   
					s = va_arg(ap, const char*);
                    for ( ; *s; s++)
                    {
						USART_SendData(USARTx,*s);
						while( USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET );
					}				
				    Data++;
				    break;
				
				case 'd':                           //十进制
					d = va_arg(ap, int);
				    itoa(d, buf, 10);
			     	for (s = buf; *s; s++) 
					{
						USART_SendData(USARTx,*s);
						while( USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET );
					}
				    Data++;
				    break;
				
				default:
					Data++;
				    break;
			}
		}
		
		else
		{
			USART_SendData(USARTx, *Data++);
		}
		while( USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET );
	}
#endif
}

/*************************************************************
*Function Name  : USART3_Send_Data
*Auther         : 张沁
*Version        : V1.0
*Date           : 2017-04-19
*Description    : UART3数据发送
*Input          ：1. *data 要发送到内容的指针
                  2. len   数据长度
*Output         ：
*Return         ：
*************************************************************/
void USART3_Send_Data(u8 *data, u8 len)
{
	u8 i=0;
	
	while( USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET );
	
	for(i = 0; i < len; i++)
	{
		USART_SendData(USART3,	data[i]);
		while(RESET == USART_GetFlagStatus(USART3, USART_FLAG_TC));
	}
}

/*************************************************************
*Function Name  : USART3_IRQHandler
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-15
*Description    : USART3的中断服务例程，包含了传输协议（以回车换行符结束）
*Input          ：
*Output         ：
*Return         ：
*Histoty        :
1. Date: 2017-04-15    Auther: 张沁
   Modifycation: 添加uCOS-II兼容程序
*************************************************************/
void USART3_IRQHandler(void)
{
	char ch;
	
#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif
	
	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)     //如果接收寄存器不为空
	{
		ch  = USART_ReceiveData( USART3 );
		
		if (Wifi_Data_Buf.Wifi_Data_Len < ( RX_BUF_MAX_LEN-1) )//预留一个字节写结束符
		{
			Wifi_Data_Buf.Wifi_Data_Buffer[Wifi_Data_Buf.Wifi_Data_Len++] = ch;
		}
	}
	
	if ( USART_GetITStatus(USART3, USART_IT_IDLE) == SET )     //数据帧接收完毕
	{
		if (Wifi_Data_Buf.Wifi_Data_Buffer[0] != 0xff)         //不明白！0xff可能是汉字库里面的字符
		{
			/***非连接请求状态下***/
			if(startFlag == 0)
			{
				if (Wifi_Data_Buf.Wifi_Data_Buffer[0] == 'H')  //如果是服务器回传数据包 
				{
					OSSemPost(semWifiData);                    //发送 数据包信号量
				}
				else
				{
					OSSemPost(semWifi);                        //发送 AT指令信号量
				} 
			}
			/***连接请求状态下***/
			else
			{
				if (Wifi_Data_Buf.Wifi_Data_Buffer[Wifi_Data_Buf.Wifi_Data_Len-3]=='P')//连接成功（WIFI GOT IP）
				{
					OSSemPost(semWifi);                        //发送信号量
				}
			}
		}
		
		Wifi_Data_Buf.Wifi_Data_Rev_Finish = 1;
		

		USART_ReceiveData(USART3); //读DR  由软件序列清除中断标志位
		                           //(先读USART_SR，然后读USART_DR)
	}
	
#if SYSTEM_SUPPORT_OS 	           //如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif
}
