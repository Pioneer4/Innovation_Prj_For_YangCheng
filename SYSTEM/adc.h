/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : adc.h
*Auther         : ����
*Vertion        : v1.0
*Date           : 2017-10-10
*Description    : ���ڶ�adc�ӿڵĳ�ʼ����Դ������ԭ�ӣ�����Ӧ�޸ģ���
*Function List  : 
*************************************************************/
#include "myinclude.h" 

#ifndef __ADC_H_
#define __ADC_H_	

void AdcInit(void); 				//ADCͨ����ʼ��
u16  GetAdc(u8 ch); 				//���ĳ��ͨ��ֵ 
u16 GetAdcAverage(u8 ch,u8 times);//�õ�ĳ��ͨ����������������ƽ��ֵ  

#endif 















