/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : key.h
*Auther         : 张沁
*Version        : V2.0
*Date           : 2017-04-16
*Description    : 该文件包含了 key.c中所有的函数原型
*Function List  : 
*Histoty		:
1. Date: 2017-04-15    Auther: 张沁
   Modifycation: 添加uCOS-II兼容程序
*************************************************************/

#ifndef __KEY_H
#define __KEY_H	 
#include "myinclude.h" 

#define GET_KEY0_STATUS() PEin(4)
#define KEY0_ON   0
#define KEYO_OFF  1

/*下面的方式是通过直接操作库函数方式读取IO*/
#define KEY0 		GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4) //PE4
#define KEY1 		GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3)	//PE3 
#define KEY2 		GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2) //PE2
#define WK_UP 	GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)	//PA0

void KeyInit(void);

#endif
