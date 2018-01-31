/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : dht11.h
*Auther         : 张沁
*Version        : v1.0
*Date           : 2016-05-03
*Description    : 该文件包含了dht11.c中所有的函数原型
*Function List  : 
*************************************************************/
#ifndef __DHT11_H
#define __DHT11_H 

#include "myinclude.h"

//IO方向设置
#define DHT11_IO_IN()  {GPIOE->MODER &= ~(3<<(2*2)); GPIOE->MODER |= 0<<2*2;}//PE2输入模式
#define DHT11_IO_OUT() {GPIOE->MODER &= ~(3<<(2*2)); GPIOE->MODER |= 1<<2*2;}//PE2输出模式 
//IO操作函数											   
#define	DHT11_DQ_OUT PEout(2) //数据端口	PE2 
#define	DHT11_DQ_IN  PEin(2)  //数据端口	PE2 

void DHT11_Init(void);                  //初始化DHT11
void DHT11_Rst(void);                 //复位DHT11
u8 DHT11_Check(void);                 //检测是否存在DHT11
u8 DHT11_Read_Bit(void);              //读出一个位
u8 DHT11_Read_Byte(void);             //读出一个字节
u8 DHT11_Read_Data(u8 *temp,u8 *humi);//读取温湿度

#endif
