/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : led.c
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-16
*Description    : 该文件包含了LED小灯的相关函数
*Function List  : 
*************************************************************/

#include "myinclude.h"


/*************************************************************
*Function Name  : LedInit
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-16
*Description    : 初始化LED2(PA6)和LED3(PA7)
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void LedInit(void)
{
	GPIO_InitTypeDef GPIOA_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	GPIOA_InitStructure.GPIO_Mode = GPIO_Mode_OUT;           //输出模式
	GPIOA_InitStructure.GPIO_OType = GPIO_OType_PP;          //推挽输出
	GPIOA_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;  //GPIOA6和GPIOA7
	GPIOA_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;            //上拉
	GPIOA_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;      //速度100M
	
	GPIO_Init(GPIOA, &GPIOA_InitStructure);                  //初始化PA6，PA7
	
	GPIO_SetBits(GPIOA, GPIO_Pin_6 | GPIO_Pin_7);            //电平拉高，小灯熄灭
}

/*************************************************************
*Function Name  : Led2Switch
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-16
*Description    : 控制LED2(PA6)的亮和灭
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void Led2Switch(BitAction State)
{
	if(State == ON)        //ON  打开小灯2
	{
		LED2 = 0;
	}
		
	else if(State == OFF)  //OFF 关闭小灯2
	{
		LED2 = 1;
	}
}


/*************************************************************
*Function Name  : Led3Switch
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-05-01
*Description    : 控制LED2(PA7)的亮和灭
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void Led3Switch(BitAction State)
{
	if(State == ON)        //ON  打开小灯2
	{
		LED3 = 0;
	}
		
	else if(State == OFF)  //OFF 关闭小灯2
	{
		LED3 = 1;
	}
}


/*************************************************************
*Function Name  : ClassroomLedInit
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-10-11
*Description    : 初始化教室的四盏LED
                  PE6, PE7, PE8， PE9
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void ClassroomLedInit(void)
{
	GPIO_InitTypeDef GPIOB_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
	GPIOB_InitStructure.GPIO_Mode = GPIO_Mode_OUT;           //输出模式
	GPIOB_InitStructure.GPIO_OType = GPIO_OType_PP;          //推挽输出
	GPIOB_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;  
	GPIOB_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;            //上拉
	GPIOB_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;      //速度100M
	GPIO_Init(GPIOB, &GPIOB_InitStructure);         
	
	GPIO_SetBits(GPIOB, GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9);//电平拉高，小灯熄灭
}


/*************************************************************
*Function Name  : ClassroomLedxSwitch
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-10-11
*Description    : 初始化教室的四盏LED的亮灭
				  1.number: 0~3  第几盏灯
			      2.State :  ON: 亮    OFF: 灭
*Input          ：
*Output         ：
*Return         ：
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
