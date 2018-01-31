/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : infraded.c
*Auther         : ����
*Vertion        : v1.0
*Date           : 2017-10-08
*Description    : ���ļ������˺��⴫����(infraded sensor)�ĳ�ʼ��
*Function List  : 
*************************************************************/

#include "myinclude.h"

#if SYSTEM_SUPPORT_OS
#include "includes.h"  //ucos ʹ��	  
#endif

extern u8 infradedFlag;
extern u8 peopelNum;

/*************************************************************
*Function Name  : InfradedInit
*Auther         : ����
*Vertion        : v1.0
*Date           : 2017-10-08
*Description    : ��ʼ�����⴫�����������
                  PC1~PC4 ����ȡ�������������ŵ�ƽ
                  PE0��PE1����ȡ����Թ�������ŵ�ƽ
*Input          ��
*Output         ��
*Return         ��
*************************************************************/
void InfradedInit(void)
{
	GPIO_InitTypeDef GPIOC_InitStructure;
	GPIO_InitTypeDef GPIOE_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);  //ʹ��APB2ʱ��
	
	/************��������������************/
	GPIOC_InitStructure.GPIO_Mode = GPIO_Mode_IN;          //����ģʽ
	GPIOC_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;
	GPIOC_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;        //����
	GPIOC_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;     //�ٶ�50M
	GPIO_Init(GPIOC, &GPIOC_InitStructure);                //��ʼ��PC1
	
	/************2������Թ��������************/
	GPIOE_InitStructure.GPIO_Mode = GPIO_Mode_IN;          //����ģʽ
	GPIOE_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;//GPIOE0,GPIOE1
	GPIOE_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;          //���� ��Ϊ�����ڵ�������ߵ�ƽ
	GPIOE_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;     //�ٶ�50M
	GPIO_Init(GPIOE, &GPIOE_InitStructure);                //��ʼ��PE0��PE1
	
	/******����EXTI4�ж��߿�����***********/
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource0);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource1);
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line0|EXTI_Line1;  //�ⲿ�ж���0,1
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;              //ʹ��
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;    //�ж��¼�
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//�½��ش���
	EXTI_Init(&EXTI_InitStructure);
	
	/******����Ƕ�������жϿ�����***********/
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;  //��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;         //�����ȼ�2
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;  //��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;         //�����ȼ�3
	NVIC_Init(&NVIC_InitStructure);
}

/*************************************************************
*Function Name  : EXTI0_IRQHandler
*Auther         : ����
*Version        : V1.0
*Date           : 2017-10-11
*Description    : ����Թ�0���жϷ������� 
*Input          ��
*Output         ��
*Return         ��
*************************************************************/
void EXTI0_IRQHandler(void)
{	
	//char retValue[60] = {0};
#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif
	
	EXTI_ClearFlag(EXTI_Line0);  //���Line4�ϵı�־λ
	
	if ((infradedFlag&0x02) == 0)
	{
		if (peopelNum != 0)
		{
		    peopelNum--;
		}
		infradedFlag |= 0x02;
	}
	else
	{
		infradedFlag &= ~(0x01);
	}
	
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif
}

/*************************************************************
*Function Name  : EXTI1_IRQHandler
*Auther         : ����
*Version        : V1.0
*Date           : 2017-10-11
*Description    : ����Թ�2���жϷ������� 
*Input          ��
*Output         ��
*Return         ��
*************************************************************/
void EXTI1_IRQHandler(void)
{	
	//char retValue[60] = {0};
#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif
	
	EXTI_ClearFlag(EXTI_Line1);  //���Line4�ϵı�־λ
	
	if ((infradedFlag&0x01) == 0)
	{
		if (peopelNum != 255)
		{
		    peopelNum++;
		}
		infradedFlag |= 0x01;
	}
	else
	{
		infradedFlag &= ~(0x02);
	}
	
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif
}

/*************************************************************
*Function Name  : InfradedRead
*Auther         : ����
*Version        : V1.0
*Date           : 2017-10-11
*Description    : ��ȡ�����Ӧģ��ĵ�ƽ 
*Input          ��pinx: 0~3 Ҫ��ȡ���������ģ����
*Output         ��
*Return         ��1�� �ߵ�ƽ ����   0���͵�ƽ ����
*************************************************************/
u8 InfradedRead(u8 pinx)
{
	switch (pinx)
	{
		case 0: return INFRADED0_PIN;
		case 1: return INFRADED1_PIN;
		case 2: return INFRADED2_PIN;
		case 3: return INFRADED3_PIN;
		default: break;
	}
	return 0;
}
