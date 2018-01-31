/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : iwdg.c
*Auther         : 张沁
*Version        : v1.0
*Date           : 2017-04-16
*Description    : 给文件用于独立看门狗的初试化
*Function List  : 
*************************************************************/

#include "myinclude.h"

/*************************************************************
*Function Name  : iwdgInit
*Auther         : 张沁
*Version        : v1.0
*Date           : 2016-09-07
*Description    : 用于IWDG的初始化
*Input          ：1. prr 分频系数 分频 = 4*2^prr  prr(0~6)
                  2. rlr 重装载值 0~65535
*Output         ：
*Return         ：
*************************************************************/
void IwdgInit(u8 prr, u16 rlr)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); //使能对 IWDG_PR 和 IWDG_RLR 寄存器的访问
	IWDG_SetPrescaler(prr);                       //设置预分频值
	IWDG_SetReload(rlr);                          //设置重装载值
	IWDG_Enable();                                //看门狗使能
	IWDG_ReloadCounter();                         //重新加载重装载值
}

/*************************************************************
*Function Name  : IwdgFeed
*Auther         : 张沁
*Version        : v1.0
*Date           : 2017-04-16
*Description    : IWDG的喂狗操作
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void IwdgFeed(void)
{

	IWDG_ReloadCounter();
}
