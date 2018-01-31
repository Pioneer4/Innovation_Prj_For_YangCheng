/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : led.c
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-16
*Description    : 该文件包含了led.c中所有的函数原型
*Function List  : 
*************************************************************/
#ifndef __LED_H
#define __LED_H

#include "myinclude.h"

//LED端口定义
#define LED2 PAout(6)
#define LED3 PAout(7)	

#define CLED0 PBout(6)
#define CLED1 PBout(7)
#define CLED2 PBout(8)
#define CLED3 PBout(9)

#define LED_STATUS_ON   0
#define LED_STATUS_OFF  1
#define GET_LED2_STATUS() PAin(6)

#define ON  Bit_RESET
#define OFF Bit_SET

void LedInit(void);         //初始化板载LED
void Led2Switch(BitAction State);
void Led3Switch(BitAction State);

void ClassroomLedInit(void);//初始化教室的四盏LED控制接口
void ClassroomLedxSwitch(u8 number, BitAction state);

#endif
