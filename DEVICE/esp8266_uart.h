/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : esp8266_uart.h
*Auther         : 张沁
*Version        : v1.0
*Date           : 2017-04-19
*Description    : 该文件包含了esp8266_uart.cz中所有的函数原型
*Function List  : 
*************************************************************/

#ifndef __ESP8266_UART_H
#define	__ESP8266_UART_H


#include "myinclude.h"


void UART3_Init(u32 bound);
static char *itoa( int value, char *string, int radix );
void USART3_printf(USART_TypeDef* USARTx, char *Data, ...);
void USART3_Send_Data(u8 *data, u8 len);


#endif 
