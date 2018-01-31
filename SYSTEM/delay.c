/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : delay.c
*Auther         : 张沁
*Version        : v2.0  兼容uCOS-II
*Date           : 2017-04-15
*Description    : 用systick实现较精准的延时
*Function List  : 
*Histoty		:
1. Date: 2017-04-15    Auther: 张沁
   Modifycation: 添加uCOS-II兼容程序
2. Date: 2017-05-16    Auther: 张沁
   Modifycation: 修改了代码在非uCOS的工程中，延时出错的BUG
*************************************************************/
#include "myinclude.h"
#include "sys.h"

#if SYSTEM_SUPPORT_OS
#include "includes.h"	//支持OS时，使用	  
#endif

static u8 fac_us = 0;   //us延时被乘数
static u16 fac_ms = 0;  //ms延时被乘数，在os下，代表每个节拍的ms数

#if SYSTEM_SUPPORT_OS

#ifdef 	OS_CRITICAL_METHOD						//OS_CRITICAL_METHOD定义了,说明要支持UCOSII				
#define delay_osrunning		OSRunning			//OS是否运行标记,0,不运行;1,在运行
#define delay_ostickspersec	OS_TICKS_PER_SEC	//OS时钟节拍,即每秒调度次数
#define delay_osintnesting 	OSIntNesting		//中断嵌套级别,即中断嵌套次数
#endif

/*************************************************************
*Function Name  : delay_osschedlock
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-15
*Description    : us级延时时,关闭任务调度(防止打断us级延迟)
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void delay_osschedlock(void)
{
	OSSchedLock();  //禁止调度，防止打断us延时
}

/*************************************************************
*Function Name  : delay_osschedunlock
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-15
*Description    : us级延时时,恢复任务调度
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void delay_osschedunlock(void)
{
	OSSchedUnlock();  //恢复调度
}

/*************************************************************
*Function Name  : delay_ostimedly
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-15
*Description    : 调用OS自带的延时函数延时
*Input          ：1. ticks  延时的节拍数
*Output         ：
*Return         ：
*************************************************************/
void delay_ostimedly(u32 ticks)
{
	OSTimeDly(ticks);						//UCOSII延时
}

/*************************************************************
*Function Name  : SysTick_Handler
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-15
*Description    : systick中断服务函数,使用OS时用到
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void SysTick_Handler(void)
{
	if (delay_osrunning == 1)
	{
		OSIntEnter();						//进入中断
		OSTimeTick();       				//调用ucos的时钟服务程序               
		OSIntExit();       	 				//触发任务切换软中断
	}
}

#endif

/*************************************************************
*Function Name  : delay_init
*Auther         : 张沁
*Vertion        : v2.0 代码兼容uCOS-II
*Date           : 2017-04-15
*Description    : SysTick初始化（时钟源选择为外部时钟）
*Input          ：1. SYSCLK 系统时钟频率 单位:M
*Output         ：
*Return         ：
*************************************************************/
void delay_init(u8 SYSCLK)
{
#if SYSTEM_SUPPORT_OS  //如果需要支持OS.
	u32 reload;
#endif
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
	fac_us = SYSCLK / 8;
	
#if SYSTEM_SUPPORT_OS
	reload = SYSCLK / 8;                        //每秒的计数次数 单位：M
	reload *= 1000000 / delay_ostickspersec;    //根据OS时钟节拍设定溢出时间
	                                            //reload为24位寄存器,最大值:16777216,在168M下,约合0.7989s左右
	fac_ms = 1000 / delay_ostickspersec;        //代表OS可以延时的最少单位  1000ms/OS_TICKS_PER_SEC
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;  //开启SYSTICK中断
	SysTick->LOAD = reload;                     //每 1/delay_ostickspersec 秒中断一次
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;   //开启SYSTICK
	
#else
	fac_ms = (u16)fac_us * 1000;                 //非OS下,代表每个ms需要的systick时钟数
	
#endif
}

#if SYSTEM_SUPPORT_OS 						//如果需要支持OS

/*************************************************************
*Function Name  : delay_us
*Auther         : 张沁
*Vertion        : v2.0 代码兼容uCOS-II
*Date           : 2017-04-15
*Description    : 实现us延时
*Input：        : 1.nus 延时的us数 不要大于798915us
*Output         ：
*Return         ：
*************************************************************/
void delay_us(u32 nus)
{		
	u32 ticks;
	u32 told,tnow,tcnt = 0;
	u32 reload = SysTick->LOAD;	 //LOAD的值	    	 
	ticks = nus * fac_us;  	     //需要的节拍数 
	delay_osschedlock();		 //阻止OS调度，防止打断us延时
	told = SysTick->VAL;       	 //刚进入时的计数器值
	while(1)
	{
		tnow=SysTick->VAL;	
		if (tnow != told)
		{	    
			if (tnow < told) tcnt+=told-tnow;//这里注意一下SYSTICK是一个递减的计数器就可以了.
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if (tcnt >= ticks) break;		 //时间超过/等于要延迟的时间,则退出.
		}  
	};
	delay_osschedunlock();					 //恢复OS调度											    
} 

/*************************************************************
*Function Name  : delay_ms
*Auther         : 张沁
*Vertion        : v2.0 代码兼容uCOS-II
*Date           : 2017-04-15
*Description    : 实现us延时
*Input：        : 1.nms 延时的ms数 0~65535
*Output         ：
*Return         ：
*************************************************************/
void delay_ms(u16 nms)
{	
    if (delay_osrunning && delay_osintnesting == 0)//如果OS已经在跑了,并且不是在中断里面(中断里面不能任务调度)	    
	{		 
		if (nms>=fac_ms)						   //延时的时间大于OS的最少时间周期 
		{ 
   			delay_ostimedly(nms/fac_ms);           //OS延时
		}
		nms %= fac_ms;						       //OS已经无法提供这么小的延时了,采用普通方式延时    
	}
	delay_us((u32)(nms*1000));				       //普通方式延时（阻止OS调度，防止打断us延时）
}

#else
/************************************************************/
//Function Name: delay_xms
//Auther: 张沁
//Vertion: v1.0
//Date: 2016-09-01
//Description: 实现ms的延时
//Input：1.nms 延时的ms数 nms <= 798ms
//Output：
//Return：
/************************************************************/
void delay_xms(u16 nms)
{
	u32 temp;
	
	SysTick->LOAD = (u32) nms * fac_ms;        //时间加载值
	SysTick->VAL = 0x00;
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;  //开始计数
	
	do
	{
		temp = SysTick->CTRL;
	}
	while((temp & 0x01) && !(temp & (1<<16)));
	
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; //关闭计数器
	SysTick->VAL = 0x00;  //清空计数器
}

/************************************************************/
//Function Name: delay_us
//Auther: 张沁
//Vertion: v1.0
//Date: 2016-08-31
//Description: 实现us延时
//Input：1.nus 延时的us数 不要大于798915us
//Output：
//Return：
/************************************************************/
void delay_us(u32 nus)
{
	u32 temp;
	SysTick->LOAD = (u32) nus * fac_us;        //时间加载值
	SysTick->VAL = 0x00;                       //清空计数器
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;  //开始倒数
	
	do
	{
		temp = SysTick->CTRL;
	}
	while((temp & 0x01) && !(temp & (1<<16))); //等待时间到达
	
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; //关闭计数器
	SysTick->VAL = 0x00;                       //清空计数器
}

/************************************************************/
//Function Name: delay_ms
//Auther: 张沁
//Vertion: v1.0
//Date: 2016-08-31
//Description: 实现ms延时
//Input：1.nms 延时的ms数 0~65535
//Output：
//Return：
/************************************************************/
void delay_ms(u16 nms)
{
	u8 repeat = nms / 540;  //这里用540,是考虑到某些客户可能超频使用,
						    //比如超频到248M的时候,delay_xms最大只能延时541ms左右了
	
	u16 remain = nms % 540;
	
	while(repeat)
	{
		delay_xms(540);
		repeat--;
	}
	
	if(remain)
	{
		delay_xms(remain);
	}
}
#endif
