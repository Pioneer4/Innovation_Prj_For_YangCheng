/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : esp826.h
*Auther         : 张沁
*Version        : v1.0
*Date           : 2017-04-19
*Description    : 该文件包含了esp8266.c中所有的函数原型
*Function List  : 
*************************************************************/

#ifndef __ESP8266_H
#define	__ESP8266_H


#include "myinclude.h"

//#define API_KEY     "EPUF6JFF7TPW1B4Zg8M69Axgk1Q= "		//需要定义为用户自己的参数
//#define DEV_ID      "4602551"							//需要定义为用户自己的参数

#define API_KEY     "=8YYu=H3SZAZ3=Winl1AXDap2tE= "		//需要定义为用户自己的参数
#define DEV_ID      "5465273"							//需要定义为用户自己的参数

#define RX_BUF_MAX_LEN     2048 
#define WIFI_MAX_DATA_LEN  600

#define AT_SEND_OK 		   1
#define AT_SEND_ERROR 	   0


#define WIFI_INIT_FAIL	   0
#define WIFI_CONNECT 	   1
#define WIFI_NOT_CONNECT   2

#define TCP_CONNECT 	   1
#define TCP_NOT_CONNECT    2

#define POWER_ON           1
#define POWER_OFF          0

#define REV_AT 		       1
#define	REV_DATA	       0

//定义WIFI数据结构体
typedef struct Gprs_Data_St
{
	u8 	Wifi_Data_Buffer[WIFI_MAX_DATA_LEN];
	u16	Wifi_Data_Len;
	u8	Wifi_Data_Rev_Finish;
}WIFI_Data;

u8   ESP8266_Init(void);
void ESP8266_Power_Pin_Init(void);
void ESP8266_Power_Ctrl(u8 powerCtrl);

u8	 SendCmd(char *cmd, char *reply1, char *reply2, char *ret_value, u32 waittime);

void ESP8266_Con_Udp(u8 wifiConnectState);



#endif 
