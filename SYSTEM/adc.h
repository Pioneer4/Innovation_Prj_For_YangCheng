/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : adc.h
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-10-10
*Description    : 用于对adc接口的初始化（源于正点原子，部分应修改！）
*Function List  : 
*************************************************************/
#include "myinclude.h" 

#ifndef __ADC_H_
#define __ADC_H_	

void AdcInit(void); 				//ADC通道初始化
u16  GetAdc(u8 ch); 				//获得某个通道值 
u16 GetAdcAverage(u8 ch,u8 times);//得到某个通道给定次数采样的平均值  

#endif 















