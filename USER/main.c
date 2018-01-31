/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : main.c
*Auther         : 张沁
*Version        : v1.0
*Date           : 2016-11-10
*Description    : 主函数入口，完成UC/OS-II相关项目
*Function List  : 
*History        :
1. Date: 2017-04-23  Auther: 张沁
Modification: 添加esp8266 AT指令 成功连接WIFI及OneNet服务器
2. Date: 2017-05-02  Auther: 张沁
Modification: 添加触摸屏（XPT2046）驱动代码
2. Date: 2017-05-03  Auther: 张沁
Modification: 添加DHT11驱动代码  电机驱动代码 语音模块代码
*************************************************************/
/*
*项目描述:
*基于UCOS操作系统，完成三个任务，
*任务一用于定时的点灯和喂狗
*任务二检测并响应按键
*任务三用于在显示屏上显示任务一和任务二的状态，比如任务中LED的状态，任务二中按键的状态。
*/

#include "myinclude.h"
#include "includes.h"

OS_EVENT *semWifi;    //信号量：接收到ESP8266发送数据
OS_EVENT *semWifiData;//信号量：接收到服务器数据

/*
事件标志组：触摸屏相关
BIT0: 触摸屏是否开始扫描触摸点 
*/
OS_FLAG_GRP *flagTP; 

u8 controlState = 0;  //控制模式  0：自动  1：手动

/*
灯光标志：bit0~bit3: 分别第1~4盏灯
亮灭状态：1：灭         0：亮
*/
u8 ledFlag = 0x0F;  

/*
红外对管标志：bit0,bit1: 第0,1号红外对管
电平状态：1：无遮挡物   0：有遮挡物
*/
u8 infradedFlag = 0x03; 

u8 peopelNum = 0;     //教室总人数
u8 ledNum = 0;        //教室开灯总数

u16 fanSpeed = 0;     //风扇转速

u8 wifiConnectState;  //标志：WIFI连接是否成功 
u8 TCP_ConnectState;  //标志：TCP连接是否成功
u8 regProcess;        //标志：是否处理注册包ACK

INT8U temperature = 0;  	    
INT8U humidity = 0; 
extern u16 CO2_value;

extern WIFI_Data  Wifi_Data_Buf;


/********************************************
任务        ：WifiRevTask  wifi数据接收任务
任务优先级  ：8
任务堆栈大小：128
任务堆栈    ：wifiRevTaskStk[WIFI_REV_STK_SIZE]
*********************************************/
#define WIFI_REV_TASK_PRIO    8
#define WIFI_REV_STK_SIZE  	  128
OS_STK wifiRevTaskStk[WIFI_REV_STK_SIZE];
void WifiRevTask(void *pdata);


/********************************************
任务        ：WifiTrmTask   wifi数据发送任务
任务优先级  ：9
任务堆栈大小：256
任务堆栈    ：wifiTrmTaskStk[WIFI_TRM_STK_SIZE]
*********************************************/
#define WIFI_TRM_TASK_PRIO        9
#define WIFI_TRM_STK_SIZE  	      256            //!128 就不行
OS_STK wifiTrmTaskStk[WIFI_TRM_STK_SIZE];
void WifiTrmTask(void *pdata);


/********************************************
任务        :UartTask     UART1输出任务
任务优先级  :10
任务堆栈大小:128
任务堆栈    :uartTaskStk[KEYO_STK_SIZE]
*********************************************/
#define UART_TASK_PRIO       10  	  
#define UART_STK_SIZE  		 128
OS_STK uartTaskStk[UART_STK_SIZE];
void UartTask(void *pdata);


/********************************************
任务        ：ControlTask
任务优先级  ：11
任务堆栈大小：128
任务堆栈    ：ControlTaskStk[Control_STK_SIZE]
*********************************************/
#define Control_TASK_PRIO    11
#define Control_STK_SIZE     128
OS_STK ControlTaskStk[Control_STK_SIZE];
void ControlTask(void *pdata);

/********************************************
任务        ：LcdShowTask  LCD显示屏任务
任务优先级  ：14
任务堆栈大小：128
任务堆栈    ：lcdShowTaskStk[LCD_SHOW_STK_SIZE]
*********************************************/
#define LCD_SHOW_TASK_PRIO   14    	  
#define LCD_SHOW_STK_SIZE    128  		    	
OS_STK lcdShowTaskStk[LCD_SHOW_STK_SIZE];
void LcdShowTask(void *pdata);


/********************************************
任务        ：TP_Task    TP触摸屏任务
任务优先级  ：15
任务堆栈大小：256
任务堆栈    ：tpTaskStk[TP_STK_SIZE];
*********************************************/
#define TP_TASK_PRIO         15
#define TP_STK_SIZE  	     128
OS_STK tpTaskStk[TP_STK_SIZE];
void TP_Task(void *pdata);

/********************************************
任务        ：START 进行其它任务设置
任务优先级  ：16
任务堆栈大小：256
任务堆栈    ：startTaskSTK[START_STK_SIZE]
*********************************************/
#define START_TASK_PRIO      16
#define START_STK_SIZE  	 128
OS_STK startTaskSTK[START_STK_SIZE];
void StartTask(void *pdata);


/*************************************************************
*Function Name  : IwdgTmrCallBack
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2016-12-06
*Description    : 定时器回调函数，用于喂狗和小灯的闪烁
*Input          : 1. void *ptmr
                  2. void *parg
				  没有传递的参数，则无需操作
*Output         :
*Return         :
*************************************************************/
OS_TMR *pTmrIwdg;
void IwdgTmrCallBack(void *ptmr, void *parg)
{
	if(GET_LED2_STATUS() == LED_STATUS_ON)
	{
        Led2Switch(OFF);
	}
	else
	{
		Led2Switch(ON);
	}
	
	IwdgFeed();  //喂狗
	
}


u8 err = 0;               //错误返回区


int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
	
	delay_init(168);      //初始化延时函数
	LedInit();		      //初始化LED端口 
	ClassroomLedInit();   //初始化教室的四盏LED控制接口
	UART1_Init(115200);   //初始化串口1
	AdcInit();            //初始化ADC接口
	LCD_Init();	          //初始化LCD	
	KeyInit();   	      //初始化按键
	InfradedInit();       //初始化红外传感器
	DcMotorInit();        //初始化直流电机
	
	OSInit();             //初始化uC/OS
	OSTaskCreate(StartTask, (void*)0, (OS_STK *)&startTaskSTK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	OSStart();	
	
	return 0;
}

/*************************************************************
*Function Name  : StartTask
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2016-12-05
*Description    : 开始任务 该任务用于建立了其它任务
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void StartTask(void *pdata)
{
	OS_CPU_SR cpu_sr = 0;
	
	pdata = pdata; 
	
	semWifi = OSSemCreate(0);	       //创建信号量	接收到ESP8266发送数据	 
	semWifiData = OSSemCreate(0);      //创建信号量	接收到服务器数据
	wifiConnectState = ESP8266_Init(); //初始化ESP8266所需接口，并获得连接状态
	
	flagTP = OSFlagCreate(0, &err);    //创建事件标志组
	
	OSStatInit();					   //初始化统计任务.这里会延时1秒钟左右
	
	OS_ENTER_CRITICAL();		       //进入临界区(无法被中断打断)    
	
	pTmrIwdg = OSTmrCreate(0, OS_TMR_CFG_TICKS_PER_SEC, OS_TMR_OPT_PERIODIC, (OS_TMR_CALLBACK)IwdgTmrCallBack, NULL, NULL, &err);
	
	if(OSTmrStart(pTmrIwdg, &err) == OS_TRUE)
	{
		printf("Iwdg Tmr start!\r\n");
		
	}
	else
	{
		printf("Iwdg Tmr err!");
	}
	
	OSTaskCreate(UartTask,(void *)0,(OS_STK *)&uartTaskStk[UART_STK_SIZE-1],UART_TASK_PRIO);
    OSTaskCreate(TP_Task,(void *)0,(OS_STK*)&tpTaskStk[TP_STK_SIZE-1],TP_TASK_PRIO);
	OSTaskCreate(WifiTrmTask,(void *)0,(OS_STK*)&wifiTrmTaskStk[WIFI_TRM_STK_SIZE-1],WIFI_TRM_TASK_PRIO);
	OSTaskCreate(WifiRevTask,(void *)0,(OS_STK*)&wifiRevTaskStk[WIFI_REV_STK_SIZE-1],WIFI_REV_TASK_PRIO);
	OSTaskCreate(ControlTask,(void *)0,(OS_STK*)&ControlTaskStk[Control_STK_SIZE-1],Control_TASK_PRIO);
	OSTaskCreate(LcdShowTask,(void *)0,(OS_STK *)&lcdShowTaskStk[LCD_SHOW_STK_SIZE-1],LCD_SHOW_TASK_PRIO);
	
	IwdgInit(4,1500);                 //IWDG的超时周期为3S
	OSTaskSuspend(START_TASK_PRIO);	  //挂起起始任务.
	OS_EXIT_CRITICAL();				  //退出临界区(可以被中断打断)
} 


/*************************************************************
*Function Name  : WifiRevTask
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-20
*Description    : wifi数据接收任务
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void WifiRevTask(void *pdata)
{
	pdata =  pdata;
	
	while(1)
	{
		OSTimeDly(OS_TICKS_PER_SEC*3);
	}		
} 


/*************************************************************
*Function Name  : WifiTrmTask
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-20
*Description    : wifi数据发送任务
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void WifiTrmTask(void *pdata)
{
	pdata =  pdata;
		
	u8 errWifiTrmTask;
	char HTTP_Buf[100];        //HTTP报文缓存区
	char charBuff[8] = {0};    //wifi数据上传缓冲区
	u8 len;
	ESP8266_Con_Udp(wifiConnectState);
	
	while(1)
	{
		OSSemPend(semWifiData, OS_TICKS_PER_SEC*10, &errWifiTrmTask);
		memset(&Wifi_Data_Buf, 0, sizeof(Wifi_Data_Buf));//清空结构体Wifi_Data_Buf
		
		/******发送温湿度、CO2浓度******/
		sprintf(charBuff, "%d", CO2_value);
		len = HTTP_PostPkt(HTTP_Buf, API_KEY, DEV_ID, "CO2", charBuff);
		USART3_Send_Data((u8 *)HTTP_Buf, len);
		memset(HTTP_Buf, 0, sizeof(HTTP_Buf));           //清空HTTP_Buf
		sprintf(charBuff, "%d", temperature);
		len = HTTP_PostPkt(HTTP_Buf, API_KEY, DEV_ID, "temp", charBuff);
		USART3_Send_Data((u8 *)HTTP_Buf, len);
		memset(HTTP_Buf, 0, sizeof(HTTP_Buf));           //清空HTTP_Buf
		sprintf(charBuff, "%d", humidity);
		len = HTTP_PostPkt(HTTP_Buf, API_KEY, DEV_ID, "humi", charBuff);
		USART3_Send_Data((u8 *)HTTP_Buf, len);
		memset(HTTP_Buf, 0, sizeof(HTTP_Buf));           //清空HTTP_Buf
		
		/******发送风扇工作状态、转速******/
		if (fanSpeed == 0)
		{
			len = HTTP_PostPkt(HTTP_Buf, API_KEY, DEV_ID, "fanState", (char*)"0");
		}
		else
		{
			len = HTTP_PostPkt(HTTP_Buf, API_KEY, DEV_ID, "fanState", (char*)"1");
		}
		 USART3_Send_Data((u8 *)HTTP_Buf, len);
		 memset(HTTP_Buf, 0, sizeof(HTTP_Buf));           //清空HTTP_Buf
		
		sprintf(charBuff, "%d", fanSpeed);
		len = HTTP_PostPkt(HTTP_Buf, API_KEY, DEV_ID, "fanSpeed", charBuff);
		USART3_Send_Data((u8 *)HTTP_Buf, len);
		memset(HTTP_Buf, 0, sizeof(HTTP_Buf));           //清空HTTP_Buf
		
		/******发送开灯盏数******/
		sprintf(charBuff, "%d", ledNum);
		len = HTTP_PostPkt(HTTP_Buf, API_KEY, DEV_ID, "ledNum", charBuff);
		USART3_Send_Data((u8 *)HTTP_Buf, len);
		memset(HTTP_Buf, 0, sizeof(HTTP_Buf));           //清空HTTP_Buf
		
		/******发送教室人数******/
		sprintf(charBuff, "%d", peopelNum);
		len = HTTP_PostPkt(HTTP_Buf, API_KEY, DEV_ID, "peopelNum", charBuff);
		USART3_Send_Data((u8 *)HTTP_Buf, len);
		memset(HTTP_Buf, 0, sizeof(HTTP_Buf));           //清空HTTP_Buf
		
		
		OSTimeDly(OS_TICKS_PER_SEC*3);
	}
}


/*************************************************************
*Function Name  : UartTask
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-16
*Description    : Uart1显示任务
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void UartTask(void *pdata)
{
	pdata =  pdata;
	
	while(1)
	{
		#if 0
		printf("CPU Usage = %d\r\n", OSCPUUsage);
		printf("OSIdleCtrMax = %d\r\n", OSIdleCtrMax);
		printf("OSIdleCtr = %d\r\n", OSIdleCtr);
		printf("WIFI连接状态 %d\r\n", wifiConnectState);
		printf("OSTaskCtr = %d\r\n", OSTaskCtr);
		#endif

		OSTimeDly(OS_TICKS_PER_SEC*3);
	}
}

/*************************************************************
*Function Name  : ControlTask
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-10-09
*Description    : 控制任务
                  控制教室四盏灯的亮灭（自动模式和手动模式）
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void ControlTask(void *pdata)
{
	pdata = pdata;
	INT8U i = 0;
	
	while (1)
	{
		ledNum = 0;
		/******计算出当前的开灯总数******/
		for (i=0; i<4; i++)
		{
			if ( (ledFlag & (0x01 << i)) == 0 )
			{
				ledNum++;
			}
		}
		/******自动控制模式******/
		if (controlState == 0)
		{
			/*如果人数超过当前灯光负荷人数,则开灯*/
			if ( peopelNum > ledNum*5 )
			{
				for (i=0; i<4; i++)
				{
					if ( (ledFlag & (0x01 << i)) != 0 )
					{
						ledFlag &= ~(0x01 << i);
						i = 4;
					}
				}
			}
			/*如果人数不足，人体红外检测是否需要关灯*/
			else                      
			{
				for (i=0; i<4; i++)
				{
					/*该区域灯开着，并且没人时，则关灯*/
					if ( ((ledFlag & (0x01 << i)) == 0) && (InfradedRead(i) == 0) )
					{
						ledFlag |= (0x01 << i);//标志位置为熄灭状态
						if (ledNum != 0)
						{
							ledNum--;          //开灯数量减1					
						}
					}
				}
			}
		}
		
		/******控制小灯的亮灭******/
		for (i=0; i<4; i++)
		{
			if ( (ledFlag & (0x01 << i)) == 0 )
			{
				ClassroomLedxSwitch(i, ON);
			}
			else
			{
				ClassroomLedxSwitch(i, OFF);
			}
		}
		
		/******控制风扇的启动、停止、转速******/
		if ( CO2_value < 800)                       
		{
			Motor1SpeedOut(0);
			fanSpeed = 0;
		}
		else if ( CO2_value>=800 && CO2_value<1500) 
		{
			Motor1SpeedOut(160);
			fanSpeed = 160;
		}
		else if ( CO2_value>=1500 && CO2_value<2500)
		{
			Motor1SpeedOut(220);
			fanSpeed = 220;
		} 
		else if ( CO2_value>=2500 && CO2_value<3500)
		{
			Motor1SpeedOut(300);
			fanSpeed = 300;
		} 
		else if ( CO2_value>=3500 && CO2_value<5000)
		{
			Motor1SpeedOut(340);
			fanSpeed = 340;
		}
		else if ( CO2_value>=5000 && CO2_value<7000)
		{
			Motor1SpeedOut(360);
			fanSpeed = 360;
		}
		else if ( CO2_value>=7000 )                 
		{
			Motor1SpeedOut(399);
			fanSpeed = 399;
		}
		
		OSTimeDly(OS_TICKS_PER_SEC * 0.5);
	}
}


/*************************************************************
*Function Name  : LcdShowTask
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-01-14
*Description    : 在显示屏上显示 LED2的状态和按键的状态。
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void LcdShowTask(void *pdata)
{
	pdata =  pdata;
	
	char charBuff[8] = {0};    //数据显示缓冲区
	
	INT8U timeNum = 0;    //时间计数标志
	DHT11_Init();
	
	OSTimeDly(OS_TICKS_PER_SEC);
	while(1)
	{	
		/******每隔1S读取传感器数值 ********/
		if (timeNum == 5)
		{
			timeNum = 0;  //时间标志位置位
			 
			/*读取温湿度值*/
			DHT11_Read_Data(&temperature, &humidity); 
			
			/*读取CO2数值*/
			if(MGGetPercentage(MGRead()) == -1)
			{
				CO2_value = 400;
			}
			else
			{
				CO2_value = MGGetPercentage(MGRead());
			}
		}
		else
		{
			timeNum++;
		}
		
		/******CPU使用情况界面********/
		LCD_ShowString(56, 0, 104, 16, 16, (u8*)"CPU Usage:  %");
		LCD_ShowNum(136, 0, OSCPUUsage, 2, 16);	
		
	    /******WIFI和TCP连接情况的显示界面********/
		LCD_DrawRectangle(10, 30, 180, 90);
		LCD_ShowString(43, 33, 56, 16, 16, (u8*)"Status:");
		
		if (wifiConnectState == WIFI_CONNECT)
		{
			LCD_ShowString(15, 52, 160, 16, 16, (u8*)"WIFI CONNECTED  ");  
		}
		else if(wifiConnectState == WIFI_NOT_CONNECT)
		{
			LCD_ShowString(15, 52, 160, 16, 16, (u8*)"WIFI NOT CONNECT");  
		}
		else
		{
			LCD_ShowString(15, 52, 160, 16, 16, (u8*)"reconnect wifi......");
		}
		
		if (TCP_ConnectState == TCP_CONNECT)
		{
			LCD_ShowString(15, 71, 160, 16, 16, (u8*)"TCP CONNECT OK!");  
		}
		else
		{
			LCD_ShowString(15, 71, 160, 16, 16, (u8*)"TCP NOT CONNECT!");  
		}
		
		/******风扇转速********/
		sprintf(charBuff, "FS:%3d", fanSpeed);
		LCD_ShowString(190, 30, 48, 16, 16, (u8*)charBuff);
		
		/******温湿度界面********/
		LCD_ShowString(10, 100, 72, 16, 16, (u8*)"Temp:   C");	
        LCD_Draw_Circle(71, 104, 2);		
 	    LCD_ShowString(90, 100, 64, 16, 16, (u8*)"Humi:  %");
		LCD_ShowNum(50, 100, temperature, 2, 16);			   		   
		LCD_ShowNum(130, 100, humidity, 2, 16);		

		/******CO2显示界面********/
		sprintf(charBuff, "CO2:%5d", CO2_value);
		LCD_ShowString(165, 100, 88, 16, 16, (u8*)charBuff);		
		
		/******自动或手动控制界面********/
		LCD_ShowString(25, 123, 60, 24, 24, (u8*)"Auto ");
		LCD_ShowString(125, 123, 72, 24, 24, (u8*)"Manual");
		if (controlState == 0)
		{
			POINT_COLOR = WHITE;
			LCD_DrawRectangle(120, 120, 202, 150);
			POINT_COLOR = BLUE;
			LCD_DrawRectangle(20, 120, 90, 150);
			POINT_COLOR = RED;
		}
		else
		{
			POINT_COLOR = WHITE;
			LCD_DrawRectangle(20, 120, 90, 150);
			POINT_COLOR = BLUE;
			LCD_DrawRectangle(120, 120, 202, 150);
			POINT_COLOR = RED;
		}
		
		/******教室灯编号界面********/
		LCD_ShowString(30,  170, 1, 24, 24, (u8*)"1");
		LCD_ShowString(80,  170, 1, 24, 24, (u8*)"2");
		LCD_ShowString(130, 170, 1, 24, 24, (u8*)"3");
		LCD_ShowString(180, 170, 1, 24, 24, (u8*)"4");
		
		/******灯光状态及控制界面********/
		if ((ledFlag & 0x01) != 0)  //1号
		{
			POINT_COLOR = WHITE;
			LCD_DrawRectangle(13,  200,  47, 222);
			POINT_COLOR = RED;
			LCD_DrawRectangle(13,  240,  47, 262);
		}			
		else 
		{
			POINT_COLOR = WHITE;
			LCD_DrawRectangle(13,  240,  47, 262);
			POINT_COLOR = RED;
			LCD_DrawRectangle(13,  200,  47, 222);
		}
		if ((ledFlag & 0x02) != 0)  //2号
		{
			POINT_COLOR = WHITE;
			LCD_DrawRectangle(63,  200,  97, 222);
			POINT_COLOR = RED;
			LCD_DrawRectangle(63,  240,  97, 262);
		}			
		else 
		{
			POINT_COLOR = WHITE;
			LCD_DrawRectangle(63,  240,  97, 262);
			POINT_COLOR = RED;
			LCD_DrawRectangle(63,  200,  97, 222);
		}
		if ((ledFlag & 0x04) != 0)  //3号
		{
			POINT_COLOR = WHITE;
			LCD_DrawRectangle(113, 200, 147, 222);
			POINT_COLOR = RED;
			LCD_DrawRectangle(113, 240, 147, 262);
		}			
		else 
		{
			POINT_COLOR = WHITE;
			LCD_DrawRectangle(113, 240, 147, 262);
			POINT_COLOR = RED;
			LCD_DrawRectangle(113, 200, 147, 222);
		}
		if ((ledFlag & 0x08) != 0)  //4号
		{
			POINT_COLOR = WHITE;
			LCD_DrawRectangle(163, 200, 197, 222);
			POINT_COLOR = RED;
			LCD_DrawRectangle(163, 240, 197, 262);
		}			
		else 
		{
			POINT_COLOR = WHITE;
			LCD_DrawRectangle(163, 240, 197, 262);
			POINT_COLOR = RED;
			LCD_DrawRectangle(163, 200, 197, 222);
		}
		
		LCD_ShowString(18, 203, 24, 16, 16, (u8*)"ON ");
		LCD_ShowString(68, 203, 24, 16, 16, (u8*)"ON ");
		LCD_ShowString(118, 203, 24, 16, 16, (u8*)"ON ");
		LCD_ShowString(168, 203, 24, 16, 16, (u8*)"ON ");
		LCD_ShowString(18, 243, 24, 16, 16, (u8*)"OFF");
		LCD_ShowString(68, 243, 24, 16, 16, (u8*)"OFF");
		LCD_ShowString(118, 243, 24, 16, 16, (u8*)"OFF");
		LCD_ShowString(168, 243, 24, 16, 16, (u8*)"OFF");
		
		/******人体红外工作情况显示界面********/
		if (InfradedRead(0) == 0) Show_Circle(30, 283, 10, WHITE);
		else                      Show_Circle(30, 283, 10, BLUE);
		if (InfradedRead(1) == 0) Show_Circle(80, 283, 10, WHITE);
		else                      Show_Circle(80, 283, 10, BLUE);
		if (InfradedRead(2) == 0) Show_Circle(130, 283, 10, WHITE);
		else                      Show_Circle(130, 283, 10, BLUE);
		if (InfradedRead(3) == 0) Show_Circle(180, 283, 10, WHITE);
		else                      Show_Circle(180, 283, 10, BLUE);

		/******开灯总数显示界面********/
		sprintf(charBuff, "Led:%d", ledNum);
		LCD_ShowString(10, 300, 56, 16, 16, (u8*)charBuff);
		
		/******教室人数显示界面********/
		sprintf(charBuff, "peopel:%3d", peopelNum);
		LCD_ShowString(150, 300, 81, 16, 16, (u8*)charBuff);
		
		OSTimeDly(100);  //延时100个Ticks = 200ms 
	}
}


/*************************************************************
*Function Name  : TP_Task
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-20
*Description    : TP触摸屏任务
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void TP_Task(void *pdata)
{
	pdata =  pdata;
	u8 errTP_Task;
	
	/*********************触摸屏初始化*********************/
    tp_dev.init();		  //触摸屏初始化
	POINT_COLOR=RED;      //设置字体为红色 
	if (tp_dev.touchtype != 0xFF)
	{
		LCD_ShowString(30, 130 ,200, 16, 16, (u8*)"Press KEY0 to Adjust");
	}
	Load_Drow_Dialog();   //清空屏幕并在右上角显示"CLR"
	/******************************************************/
	
	while(1)
	{
		OSFlagPend(flagTP, 0x01, OS_FLAG_WAIT_SET_ALL + OS_FLAG_CONSUME, 0, &errTP_Task);
		tp_dev.scan(0);
		if (tp_dev.x[0]>(lcddev.width-24) && tp_dev.y[0]<16)
		{
			Load_Drow_Dialog();     //清除
			OSFlagPost(flagTP, 0x04, OS_FLAG_SET, &errTP_Task);
		}
		else if (tp_dev.x[0]>20 && tp_dev.x[0]<90 && tp_dev.y[0]>120 && tp_dev.y[0]<150)
		{
			controlState = 0;      //点击"Auto"   自动模式
		}
		else if (tp_dev.x[0]>120 && tp_dev.x[0]<202 && tp_dev.y[0]>120 && tp_dev.y[0]<150)
		{
			controlState = 1;      //点击"Manual" 手动模式
		}
		else if (tp_dev.x[0]>13 && tp_dev.x[0]<47 && tp_dev.y[0]>200 && tp_dev.y[0]<222)
		{
			if (controlState == 1) ledFlag &= 0x0E;//第一盏灯ON
		}
		else if (tp_dev.x[0]>13 && tp_dev.x[0]<47 && tp_dev.y[0]>240 && tp_dev.y[0]<262)
		{
			if (controlState == 1) ledFlag |= 0x01;//第一盏灯OFF
		}
		else if (tp_dev.x[0]>63 && tp_dev.x[0]<97 && tp_dev.y[0]>200 && tp_dev.y[0]<222)
		{
			if (controlState == 1) ledFlag &= 0x0D;//第二盏灯ON
		}
		else if (tp_dev.x[0]>63 && tp_dev.x[0]<97 && tp_dev.y[0]>240 && tp_dev.y[0]<262)
		{
			if (controlState == 1) ledFlag |= 0x02;//第二盏灯OFF
		}
		else if (tp_dev.x[0]>113 && tp_dev.x[0]<147 && tp_dev.y[0]>200 && tp_dev.y[0]<222)
		{
			if (controlState == 1) ledFlag &= 0x0B;//第三盏灯ON
		}
		else if (tp_dev.x[0]>113 && tp_dev.x[0]<147 && tp_dev.y[0]>240 && tp_dev.y[0]<262)
		{
			if (controlState == 1) ledFlag |= 0x04;//第三盏灯OFF
		}
		else if (tp_dev.x[0]>163 && tp_dev.x[0]<197 && tp_dev.y[0]>200 && tp_dev.y[0]<222)
		{
			if (controlState == 1) ledFlag &= 0x07;//第四盏灯ON
		}
		else if (tp_dev.x[0]>163 && tp_dev.x[0]<197 && tp_dev.y[0]>240 && tp_dev.y[0]<262)
		{
			if (controlState == 1) ledFlag |= 0x08;//第四盏灯OFF
		}
		else
		{
			TP_Draw_Big_Point(tp_dev.x[0], tp_dev.y[0], RED);
		}
	}
}
/*******************************代码草稿区*******************************/
#if 0


#endif

