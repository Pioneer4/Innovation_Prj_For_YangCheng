/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : led.c
*Auther         : ����
*Vertion        : v1.0
*Date           : 2017-04-16
*Description    : ���ļ�������LEDС�Ƶ���غ���
*Function List  : 
*************************************************************/

#include "myinclude.h"


/*************************************************************
*Function Name  : LedInit
*Auther         : ����
*Vertion        : v1.0
*Date           : 2017-04-16
*Description    : ��ʼ��LED2(PA6)��LED3(PA7)
*Input          ��
*Output         ��
*Return         ��
*************************************************************/
void LedInit(void)
{
	GPIO_InitTypeDef GPIOA_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	GPIOA_InitStructure.GPIO_Mode = GPIO_Mode_OUT;           //���ģʽ
	GPIOA_InitStructure.GPIO_OType = GPIO_OType_PP;          //�������
	GPIOA_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;  //GPIOA6��GPIOA7
	GPIOA_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;            //����
	GPIOA_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;      //�ٶ�100M
	
	GPIO_Init(GPIOA, &GPIOA_InitStructure);                  //��ʼ��PA6��PA7
	
	GPIO_SetBits(GPIOA, GPIO_Pin_6 | GPIO_Pin_7);            //��ƽ���ߣ�С��Ϩ��
}

/*************************************************************
*Function Name  : Led2Switch
*Auther         : ����
*Vertion        : v1.0
*Date           : 2017-04-16
*Description    : ����LED2(PA6)��������
*Input          ��
*Output         ��
*Return         ��
*************************************************************/
void Led2Switch(BitAction State)
{
	if(State == ON)        //ON  ��С��2
	{
		LED2 = 0;
	}
		
	else if(State == OFF)  //OFF �ر�С��2
	{
		LED2 = 1;
	}
}


/*************************************************************
*Function Name  : Led3Switch
*Auther         : ����
*Vertion        : v1.0
*Date           : 2017-05-01
*Description    : ����LED2(PA7)��������
*Input          ��
*Output         ��
*Return         ��
*************************************************************/
void Led3Switch(BitAction State)
{
	if(State == ON)        //ON  ��С��2
	{
		LED3 = 0;
	}
		
	else if(State == OFF)  //OFF �ر�С��2
	{
		LED3 = 1;
	}
}


/*************************************************************
*Function Name  : ClassroomLedInit
*Auther         : ����
*Vertion        : v1.0
*Date           : 2017-10-11
*Description    : ��ʼ�����ҵ���յLED
                  PE6, PE7, PE8�� PE9
*Input          ��
*Output         ��
*Return         ��
*************************************************************/
void ClassroomLedInit(void)
{
	GPIO_InitTypeDef GPIOB_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
	GPIOB_InitStructure.GPIO_Mode = GPIO_Mode_OUT;           //���ģʽ
	GPIOB_InitStructure.GPIO_OType = GPIO_OType_PP;          //�������
	GPIOB_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;  
	GPIOB_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;            //����
	GPIOB_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;      //�ٶ�100M
	GPIO_Init(GPIOB, &GPIOB_InitStructure);         
	
	GPIO_SetBits(GPIOB, GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9);//��ƽ���ߣ�С��Ϩ��
}


/*************************************************************
*Function Name  : ClassroomLedxSwitch
*Auther         : ����
*Vertion        : v1.0
*Date           : 2017-10-11
*Description    : ��ʼ�����ҵ���յLED������
				  1.number: 0~3  �ڼ�յ��
			      2.State :  ON: ��    OFF: ��
*Input          ��
*Output         ��
*Return         ��
*************************************************************/
void ClassroomLedxSwitch(u8 number, BitAction state)
{
	switch (number)
	{
		case 0:
		{
			if(state == ON)       
			{
				CLED0 = 0;
			}
			else if(state == OFF) 
			{
				CLED0 = 1;
			}
			break;
		}
		case 1:
		{
			if(state == ON)       
			{
				CLED1 = 0;
			}
			else if(state == OFF) 
			{
				CLED1 = 1;
			}
			break;
		}
		case 2:
		{
			if(state == ON)       
			{
				CLED2 = 0;
			}
			else if(state == OFF) 
			{
				CLED2 = 1;
			}
			break;
		}
		case 3:
		{
			if(state == ON)       
			{
				CLED3 = 0;
			}
			else if(state == OFF) 
			{
				CLED3 = 1;
			}
			break;
		}
		default:  
			break;
	}
}
