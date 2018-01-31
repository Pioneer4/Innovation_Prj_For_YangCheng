/************************************************************/
//     Open source but not allowed to modify
//File name: 
//Auther: 张沁
//Version: v1.0
//Date: 2016-08-25
//Description: 该文件包含了uart.c中的所有函数原型
//Function List: 
//
/************************************************************/
#ifndef  __UART_H
#define  __UART_H

#include "stm32f4xx.h"
#include "stdio.h"

#define UART_REC_LEN  200  //定义最大接收字节数 200

void UART1_Init(u32 bound);
void Clear_Serial(u8 USART_RX_BUF[]);  


#endif  //__UART_H


