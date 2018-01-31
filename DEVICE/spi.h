/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : spi.h
*Auther         : 张沁
*Version        : v1.0
*Date           : 2016-09-22
*Description    : 该文件包含了有关spi.c中的所有函数的原型
*Function List  : 
*************************************************************/
#ifndef __SPI_H
#define __SPI_H

#include "stm32f4xx.h"

 	    													  
void SPI1_Init(void);			              //初始化SPI1口
u8 SPI1_ReadWriteByte(u8 TxData);             //SPI1总线读写一个字节
void SPI1_SetSpeed(u8 SPI_BaudRatePrescaler); //设置SPI1速度   

#endif

