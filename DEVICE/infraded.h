/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : infraded.h
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-10-08
*Description    : 该文件包含infraded.c中所有的函数原型
*Function List  : 
*************************************************************/
#ifndef __INFRADED_H
#define __INFRADED_H

#include "myinclude.h"

#define INFRADED0_PIN    PCin(1)   //读取人体红外输出引脚电平
#define INFRADED1_PIN    PCin(2)   //读取人体红外输出引脚电平
#define INFRADED2_PIN    PCin(3)   //读取人体红外输出引脚电平
#define INFRADED3_PIN    PCin(4)   //读取人体红外输出引脚电平

void InfradedInit(void);
u8 InfradedRead(u8 pinx);

#endif
