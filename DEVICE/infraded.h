/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : infraded.h
*Auther         : ����
*Vertion        : v1.0
*Date           : 2017-10-08
*Description    : ���ļ�����infraded.c�����еĺ���ԭ��
*Function List  : 
*************************************************************/
#ifndef __INFRADED_H
#define __INFRADED_H

#include "myinclude.h"

#define INFRADED0_PIN    PCin(1)   //��ȡ�������������ŵ�ƽ
#define INFRADED1_PIN    PCin(2)   //��ȡ�������������ŵ�ƽ
#define INFRADED2_PIN    PCin(3)   //��ȡ�������������ŵ�ƽ
#define INFRADED3_PIN    PCin(4)   //��ȡ�������������ŵ�ƽ

void InfradedInit(void);
u8 InfradedRead(u8 pinx);

#endif
