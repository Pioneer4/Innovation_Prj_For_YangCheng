/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : infraded.c
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-10-08
*Description    : 该文件包含了红外传感器(infraded sensor)的初始化
*Function List  : 
*************************************************************/

#include "myinclude.h"

#if SYSTEM_SUPPORT_OS
#include "includes.h"  //ucos 使用	  
#endif

extern u8 infradedFlag;
extern u8 peopelNum;

/*************************************************************
*Function Name  : InfradedInit
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-10-08
*Description    : 初始化红外传感器相关引脚
                  PC1~PC4 ：读取人体红外输出引脚电平
                  PE0，PE1：读取红外对管输出引脚电平
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void InfradedInit(void)
{
	GPIO_InitTypeDef GPIOC_InitStructure;
	GPIO_InitTypeDef GPIOE_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);  //使能APB2时钟
	
	/************人体红外输出引脚************/
	GPIOC_InitStructure.GPIO_Mode = GPIO_Mode_IN;          //输入模式
	GPIOC_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;
	GPIOC_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;        //下拉
	GPIOC_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;     //速度50M
	GPIO_Init(GPIOC, &GPIOC_InitStructure);                //初始化PC1
	
	/************2个红外对管输出引脚************/
	GPIOE_InitStructure.GPIO_Mode = GPIO_Mode_IN;          //输入模式
	GPIOE_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;//GPIOE0,GPIOE1
	GPIOE_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;          //上拉 因为：无遮挡物输出高电平
	GPIOE_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;     //速度50M
	GPIO_Init(GPIOE, &GPIOE_InitStructure);                //初始化PE0，PE1
	
	/******开启EXTI4中断线控制器***********/
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource0);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource1);
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line0|EXTI_Line1;  //外部中断线0,1
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;              //使能
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;    //中断事件
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//下降沿触发
	EXTI_Init(&EXTI_InitStructure);
	
	/******配置嵌套向量中断控制器***********/
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;  //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;         //从优先级2
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;  //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;         //从优先级3
	NVIC_Init(&NVIC_InitStructure);
}

/*************************************************************
*Function Name  : EXTI0_IRQHandler
*Auther         : 张沁
*Version        : V1.0
*Date           : 2017-10-11
*Description    : 红外对管0的中断服务例程 
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void EXTI0_IRQHandler(void)
{	
	//char retValue[60] = {0};
#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif
	
	EXTI_ClearFlag(EXTI_Line0);  //清除Line4上的标志位
	
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
	
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif
}

/*************************************************************
*Function Name  : EXTI1_IRQHandler
*Auther         : 张沁
*Version        : V1.0
*Date           : 2017-10-11
*Description    : 红外对管2的中断服务例程 
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void EXTI1_IRQHandler(void)
{	
	//char retValue[60] = {0};
#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif
	
	EXTI_ClearFlag(EXTI_Line1);  //清除Line4上的标志位
	
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
	
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif
}

/*************************************************************
*Function Name  : InfradedRead
*Auther         : 张沁
*Version        : V1.0
*Date           : 2017-10-11
*Description    : 读取人体感应模块的电平 
*Input          ：pinx: 0~3 要读取的人体红外模块编号
*Output         ：
*Return         ：1： 高电平 有人   0：低电平 无人
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
