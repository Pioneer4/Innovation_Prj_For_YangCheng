/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : delay.h
*Auther         : 张沁
*Version        : v2.0  完善了uCOS-II的相关代码
*Date           : 2017-04-16
*Description    : 该文件包含了delay.c中所有的函数模型
*Function List  : 
*Histoty		:
1. Date: 2017-04-15    Auther: 张沁
   Modifycation: 添加uCOS-II兼容程序
*************************************************************/
#ifndef __DELAY_H
#define __DELAY_H 	

#include "myinclude.h"
#include "stm32f4xx.h"

void delay_osschedlock(void);
void delay_osschedunlock(void);
void delay_ostimedly(u32 ticks);
void delay_init(u8 SYSCLK);
void delay_xms(u16 nms);
void delay_us(u32 nus);
void delay_ms(u16 nms);



#endif

