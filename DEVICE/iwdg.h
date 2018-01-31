/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : iwdg.h
*Auther         : 张沁
*Version        : v1.0
*Date           : 2017-04-16
*Description    : 给文件包含了iwdg.c中所有的函数原型
*Function List  : 
*************************************************************/

#ifndef __IWDG_H
#define __IWDG_H

#include "myinclude.h"

void IwdgInit(u8 prr, u16 rlr);
void IwdgFeed(void);

#endif
