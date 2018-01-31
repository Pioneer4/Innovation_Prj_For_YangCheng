/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : esp8266.c
*Auther         : 张沁
*Version        : V1.0
*Date           : 2017-04-19
*Description    : 完成esp8266的相关操作
*Function List  : 
*Histoty		:
1. Date: 2017-04-15    Auther: 张沁
   Modifycation: 添加uCOS-II兼容程序
*************************************************************/

#include "myinclude.h"

#if SYSTEM_SUPPORT_OS
#include "includes.h"     //ucos 使用	  
#endif

#define SEND_CMD_WAIT_TIME  1000   //OS_TICKS_PER_SEC*5
#define MAX_RET_VALUE_LEN   50
WIFI_Data  Wifi_Data_Buf;

/*
连接请求标志
正处于连接请求状态：1
连接请求结束      ：0
*/
u8	startFlag;        

extern OS_EVENT *semWifi;
extern OS_EVENT *semWifiData;
extern u8 wifiConnectState;  //标志：WIFI连接是否成功
extern u8 TCP_ConnectState;  //标志：TCP连接是否成功

/*************************************************************
*Function Name  : ESP8266_Init
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-20
*Description    : 完成Esp8266模块初始化
				  Rx   : PB10
				  Tx   : PB11
				  CH_PD: PB12
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
u8 ESP8266_Init(void)
{
	u8 errESP8266Init;
	startFlag = 1;
	
	ESP8266_Power_Pin_Init();      //初始化后处于掉电状态
	UART3_Init(115200);
	ESP8266_Power_Ctrl(POWER_ON);  //模块上电
	
	OSSemPend(semWifi, OS_TICKS_PER_SEC*10, &errESP8266Init);
	
	/*********判断是否已经连上WIFI*********/
	if (errESP8266Init == OS_ERR_NONE)
	{
		startFlag = 0;
		
		Wifi_Data_Buf.Wifi_Data_Buffer[Wifi_Data_Buf.Wifi_Data_Len] = '\0';
		Wifi_Data_Buf.Wifi_Data_Rev_Finish = 0;
		OSTimeDly(OS_TICKS_PER_SEC*2);       //2017-05-04
		return WIFI_CONNECT;
	}
	else
	{
		startFlag = 0;
		return WIFI_INIT_FAIL;
	}
}


/*************************************************************
*Function Name  : ESP8266_Power_Pin_Init
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-20
*Description    : 完成ESP8266模块片选控制脚CH_PD的初始化 PC0
                  初始化后Esp8266模块处于掉电状态
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void ESP8266_Power_Pin_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_ResetBits(GPIOC,GPIO_Pin_0);
}


/*************************************************************
*Function Name  : ESP8266_Power_Ctrl
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-20
*Description    : 控制片选脚CH_PD 
*Input          ：1. POWER_ON  上电
                  2. POWER_OFF 掉电
*Output         ：
*Return         ：
*************************************************************/
void ESP8266_Power_Ctrl(u8 powerCtrl)
{
	if(POWER_ON == powerCtrl)
	{
		GPIO_SetBits(GPIOC,GPIO_Pin_0);
	}
	else
	{
		GPIO_ResetBits(GPIOC,GPIO_Pin_0);
	}
}


/*************************************************************
*Function Name  : ESP8266_Con_Udp
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-22
*Description    : ESP8266接入服务器 
*Input          ：wifiConnectState：
			      @arg: WIFI_CONNECT      连接成功后，进入服务器连接操作
				  @arg: WIFI_NOT_CONNECT  连接失败后，首先进入wifi连接，然后进入服务器连接
*Output         ：
*Return         ：
*************************************************************/
void ESP8266_Con_Udp(u8 wifiNowState)
{
	u8 errESP8266_Con_Udp;
	char retValue[MAX_RET_VALUE_LEN] = {0};
	
	/***由于初始化时连接WIFI(热点)失败，在这里重新发送AT指令，为连接WIFI(热点)做准备***/
	if (wifiNowState == WIFI_INIT_FAIL)
	{
		SendCmd("AT+RST", "OK", NULL, retValue, SEND_CMD_WAIT_TIME); //复位
		OSTimeDly(OS_TICKS_PER_SEC*10);                              //等待10s,过滤杂乱数据包
		OSSemSet (semWifi, 0, &errESP8266_Con_Udp);                  //清除杂乱数据包的信号量
	}
	
	SendCmd("ATE0", "OK", NULL, retValue, SEND_CMD_WAIT_TIME);
	SendCmd("AT", "OK", NULL, retValue, SEND_CMD_WAIT_TIME);
	
	/***由于初始化时连接WIFI(热点)失败，在这里重新连接WIFI(热点)，为连接服务器做准备***/
	if (wifiNowState == WIFI_INIT_FAIL)
	{
		SendCmd("AT+CWMODE=1", "OK", NULL,retValue, SEND_CMD_WAIT_TIME);
		if (AT_SEND_OK == SendCmd("AT+CWJAP=\"Pioneer's iphone\",\"zhangqin\"", "CONNECTED", NULL, retValue, SEND_CMD_WAIT_TIME*2))  //10s
		{
			wifiConnectState = WIFI_CONNECT;
		}
		else
		{
			wifiConnectState = WIFI_NOT_CONNECT;
		}
		OSTimeDly(OS_TICKS_PER_SEC*10);                              //等待10s,过滤杂乱数据包
		OSSemSet (semWifi, 0, &errESP8266_Con_Udp);                  //清除杂乱数据包的信号量
	}
	
	SendCmd("AT+CIPMUX=0", "OK", NULL,retValue, SEND_CMD_WAIT_TIME);
	SendCmd("AT+CIPMODE=1", "OK", NULL,retValue, SEND_CMD_WAIT_TIME);
	
	//if(AT_SEND_ERROR == SendCmd("AT+CIPSTART=\"TCP\",\"192.168.0.109\",8080", "CONNECT", NULL, retValue, SEND_CMD_WAIT_TIME))
	if(AT_SEND_ERROR == SendCmd("AT+CIPSTART=\"TCP\",\"183.230.40.33\",80", "CONNECT", NULL, retValue, SEND_CMD_WAIT_TIME))
	{
		TCP_ConnectState = TCP_NOT_CONNECT;
		LCD_ShowString(20,102,160,16,16,(u8*)"TCP NOT CONNECT!");
	}
	else
	{
		TCP_ConnectState = TCP_CONNECT;
	}
	SendCmd("AT+CIPSEND", "OK", NULL,retValue, SEND_CMD_WAIT_TIME);
	OSTimeDly(OS_TICKS_PER_SEC);
	memset(&Wifi_Data_Buf, 0, sizeof(Wifi_Data_Buf));  //清空结构体Wifi_Data_Buf
	OSSemPost(semWifiData);                            //发送信号量 开始上传数据
}


/*************************************************************
*Function Name  : SendCmd
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-22
*Description    : 发送一条AT指令
*Input          ：1. *cmd        AT指令
                  2. *reply1     返回值1
                  3. *reply2     返回值2
                  4. *ret_value  数据接收拷贝区
                  5. waittime    最大等待应答时间
*Output         ：
*Return         ：
*************************************************************/
u8	SendCmd(char *cmd, char *reply1, char *reply2, char *ret_value, u32 waittime)
{
	u8 errSendCmd;
	memset(&Wifi_Data_Buf, 0, sizeof(Wifi_Data_Buf));  //清空结构体Wifi_Data_Buf
	
	USART3_printf( USART3, "%s\r\n", cmd );            //发送AT指令
	
	/***不需要ACK数据***/
	if ( (reply1==NULL) && (reply2==NULL) )            
    {
		return AT_SEND_OK;
    }
	
	/***需要ACK数据***/
	if(reply1 != NULL)
	{
		OSSemPend(semWifi, waittime, &errSendCmd);     //等待接收信号 才往下执行任务
		if (errSendCmd == OS_ERR_NONE)
		{
			Wifi_Data_Buf.Wifi_Data_Rev_Finish = 0;
		}
		else
		{
			return AT_SEND_ERROR;
		}
	}
	/******************开始判断ACK是否正确******************/
	
	/***需要2个ACK数据***/
	if ( (reply1 != NULL) && (reply2 != NULL) )
	{
		if (strstr( (char *)Wifi_Data_Buf.Wifi_Data_Buffer, reply1 )!=NULL && 
			strstr( (char *)Wifi_Data_Buf.Wifi_Data_Buffer, reply2 )!=NULL ) 
		{
			return AT_SEND_OK;
		}
		else
		{
			return AT_SEND_ERROR;
		}
	}
	
	/***需要第1个ACK数据***/
	else if ( reply1 != NULL )
	{
		if (strstr( (char *)Wifi_Data_Buf.Wifi_Data_Buffer, reply1)!=NULL) 
		{
			return AT_SEND_OK;
		}
		else
		{
			return AT_SEND_ERROR;
		}
	}
	
	/***需要第二2个ACK数据***/
	else
	{
		if (strstr( (char *)Wifi_Data_Buf.Wifi_Data_Buffer, reply2)!=NULL) 
		{
			return AT_SEND_OK;
		}
		else
		{
			return AT_SEND_ERROR;
		}
	}
}
