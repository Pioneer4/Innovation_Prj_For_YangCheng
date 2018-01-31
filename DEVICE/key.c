/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : key.c
*Auther         : 张沁
*Version        : V2.0
*Date           : 2017-04-16
*Description    : 该文件对按键及按键中断进行了初始化
*Function List  : 
*Histoty		:
1. Date: 2017-04-15    Auther: 张沁
   Modifycation: 添加uCOS-II兼容程序
*************************************************************/

#include "myinclude.h"

#if SYSTEM_SUPPORT_OS
#include "includes.h"  //ucos 使用	  
#endif


/*************************************************************
*Function Name  : KeyInit
*Auther         : 张沁
*Version        : V2.0
*Date           : 2017-04-16
*Description    : 该文件包含了对按键Key0(PE4)的初始化
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void KeyInit(void)
{
	GPIO_InitTypeDef GPIO_KEY_InitStructure;
	EXTI_InitTypeDef EXTI_KEY_InitStructure;
	NVIC_InitTypeDef NVIC_KEY_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);   //使能AHB1时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);  //使能APB2时钟
	
	GPIO_KEY_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_KEY_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_KEY_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_KEY_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_KEY_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_KEY_InitStructure);
	
	/******开启EXTI4中断线控制器***********/
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource4);
	
	EXTI_KEY_InitStructure.EXTI_Line = EXTI_Line4;            //外部中断线4
	EXTI_KEY_InitStructure.EXTI_LineCmd = ENABLE;             //使能
	EXTI_KEY_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;   //中断事件
	EXTI_KEY_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;//上升沿触发
	EXTI_Init(&EXTI_KEY_InitStructure);
	
	/******配置嵌套向量中断控制器***********/
	NVIC_KEY_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_KEY_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_KEY_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;  //抢占优先级2
	NVIC_KEY_InitStructure.NVIC_IRQChannelSubPriority = 0x03;   //从优先级3
	NVIC_Init(&NVIC_KEY_InitStructure);
}

/*************************************************************
*Function Name  : EXTI4_IRQHandler
*Auther         : 张沁
*Version        : V2.0
*Date           : 2017-04-16
*Description    : KEY0的中断服务例程 
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void EXTI4_IRQHandler(void)
{	
	//char retValue[60] = {0};
#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif
	
	EXTI_ClearFlag(EXTI_Line4);  //清除Line4上的标志位
	
	//printf("中断响应成功\r\n");
	
	
	
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif
}
