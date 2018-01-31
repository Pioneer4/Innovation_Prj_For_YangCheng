/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : motor.c
*Auther         : 张沁
*Version        : v1.0
*Date           : 2017-05-03
*Description    : 该文件包含了有关减速电机引脚的初始化
*Function List  : 
*************************************************************/

#include "myinclude.h"


/*************************************************************
*Function Name  : TravelMotorInit
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-10-08
*Description    : 初始化行程电机  
                  端口：PA4 PA5
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
#if 0
void TravelMotorInit(void)
{
	GPIO_InitTypeDef GPIOA_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	GPIOA_InitStructure.GPIO_Mode = GPIO_Mode_OUT;           //输出模式
	GPIOA_InitStructure.GPIO_OType = GPIO_OType_PP;          //推挽输出
	GPIOA_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;  
	GPIOA_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;            //上拉
	GPIOA_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;      //速度100M
	
	GPIO_Init(GPIOA, &GPIOA_InitStructure);                 
}

/*************************************************************
*Function Name  : TravelMotorCtrl
*Auther         : 张沁
*Version        : v1.0
*Date           : 2017-05-03
*Description    : 用于电机运动控制
*Input          ：motorType ：
				  @arg MOTOR_FORWARD  电机前进
				  @arg MOTOR_BACKWARD 电机后退
				  @arg MOTOR_STOP     电机停止
*Output         ：
*Return         ：
*************************************************************/
void TravelMotorCtrl(u8 runType)
{
	switch (runType)
	{
		case MOTOR_FORWARD:
		{
			MOTOR1_PIN1 = 1;
			MOTOR1_PIN2 = 0;
			break;
		}
		case MOTOR_BACKWARD:
		{
			MOTOR1_PIN1 = 0;
			MOTOR1_PIN2 = 1;
			break;
		}
		case MOTOR_STOP:
		{
			MOTOR1_PIN1 = 0;
			MOTOR1_PIN2 = 0;
			break;
		}
		default:
			break;
	}
}
#endif

/*************************************************************
*Function Name  : DcMotorInit
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-08-05
*Description    : 初始化直流电机  TIM2用于生产PWM（10KHz）
                  PA0~PA1 分别对应 CH1~CH4
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void DcMotorInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);  	  //TIM2时钟使能    
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); 	  //使能PORTA时钟	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;              //复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	      //速度100MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);                    //初始化PA0,PA1,PA2,PA3
	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_TIM2);   //GPIOA0复用为定时器5
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_TIM2);   //GPIOA1复用为定时器5
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_TIM2);   //GPIOA2复用为定时器5
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_TIM2);   //GPIOA3复用为定时器5
	
	TIM_TimeBaseStructure.TIM_Prescaler = 21 - 1;             //定时器分频 84M/21 = 4M
	TIM_TimeBaseStructure.TIM_Period = 400 - 1;               //自动重装载值 4M/400 = 10KHz
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;         //选择定时器模式:TIM脉冲宽度调制模式1
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OCInitStructure.TIM_Pulse = 0;                        //设置待装入捕获比较寄存器的脉冲值
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//比较输出使能
	TIM_OC1Init(TIM2, &TIM_OCInitStructure);  
	TIM_OC2Init(TIM2, &TIM_OCInitStructure);
	TIM_OC3Init(TIM2, &TIM_OCInitStructure);  
	TIM_OC4Init(TIM2, &TIM_OCInitStructure);

	TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);  //使能TIM2在CCR1上的预装载寄存器
	TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);  //使能TIM2在CCR2上的预装载寄存器
	TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);  //使能TIM2在CCR3上的预装载寄存器
	TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);  //使能TIM2在CCR4上的预装载寄存器
 
    TIM_ARRPreloadConfig(TIM2, ENABLE);//ARPE使能 
	
	TIM_Cmd(TIM2, ENABLE);  //使能TIM2									  
}

/*************************************************************
*Function Name  : Motor1SpeedOut
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-08-05
*Description    : 电机1的PWM输出
*Input          ：motor1SpeedVal: 0~399  
                  占空比 = （motor1SpeedVal+1）/ 400
*Output         ：
*Return         ：
*************************************************************/
void Motor1SpeedOut(u16 motor1SpeedVal)
{
	TIM_SetCompare1(TIM2, motor1SpeedVal);	//修改比较值，修改占空比
}

/*************************************************************
*Function Name  : Motor2SpeedOut
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-08-05
*Description    : 电机2的PWM输出
*Input          ：motor2SpeedVal: 0~399  
                  占空比 = （motor2SpeedVal+1）/ 400
*Output         ：
*Return         ：
*************************************************************/
void Motor2SpeedOut(u16 motor2SpeedVal)
{
	TIM_SetCompare2(TIM2, motor2SpeedVal);	//修改比较值，修改占空比
}

/*************************************************************
*Function Name  : Motor3SpeedOut
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-08-05
*Description    : 电机3的PWM输出
*Input          ：motor3SpeedVal: 0~399  
                  占空比 = （motor3SpeedVal+1）/ 400
*Output         ：
*Return         ：
*************************************************************/
void Motor3SpeedOut(u16 motor3SpeedVal)
{
	TIM_SetCompare3(TIM2, motor3SpeedVal);	//修改比较值，修改占空比
}

/*************************************************************
*Function Name  : Motor4SpeedOut
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-08-05
*Description    : 电机4的PWM输出
*Input          ：motor4SpeedVal: 0~399  
                  占空比 = （motor4SpeedVal+1）/ 400
*Output         ：
*Return         ：
*************************************************************/
void Motor4SpeedOut(u16 motor4SpeedVal)
{
	TIM_SetCompare4(TIM2, motor4SpeedVal);	//修改比较值，修改占空比
}
