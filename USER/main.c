/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : main.c
*Auther         : ����
*Version        : v1.0
*Date           : 2016-11-10
*Description    : ��������ڣ����UC/OS-II�����Ŀ
*Function List  : 
*History        :
1. Date: 2017-04-23  Auther: ����
Modification: ���esp8266 ATָ�� �ɹ�����WIFI��OneNet������
2. Date: 2017-05-02  Auther: ����
Modification: ��Ӵ�������XPT2046����������
2. Date: 2017-05-03  Auther: ����
Modification: ���DHT11��������  ����������� ����ģ�����
*************************************************************/
/*
*��Ŀ����:
*����UCOS����ϵͳ�������������
*����һ���ڶ�ʱ�ĵ�ƺ�ι��
*�������Ⲣ��Ӧ����
*��������������ʾ������ʾ����һ���������״̬������������LED��״̬��������а�����״̬��
*/

#include "myinclude.h"
#include "includes.h"

OS_EVENT *semWifi;    //�ź��������յ�ESP8266��������
OS_EVENT *semWifiData;//�ź��������յ�����������

/*
�¼���־�飺���������
BIT0: �������Ƿ�ʼɨ�败���� 
*/
OS_FLAG_GRP *flagTP; 

u8 controlState = 0;  //����ģʽ  0���Զ�  1���ֶ�

/*
�ƹ��־��bit0~bit3: �ֱ��1~4յ��
����״̬��1����         0����
*/
u8 ledFlag = 0x0F;  

/*
����Թܱ�־��bit0,bit1: ��0,1�ź���Թ�
��ƽ״̬��1�����ڵ���   0�����ڵ���
*/
u8 infradedFlag = 0x03; 

u8 peopelNum = 0;     //����������
u8 ledNum = 0;        //���ҿ�������

u16 fanSpeed = 0;     //����ת��

u8 wifiConnectState;  //��־��WIFI�����Ƿ�ɹ� 
u8 TCP_ConnectState;  //��־��TCP�����Ƿ�ɹ�
u8 regProcess;        //��־���Ƿ���ע���ACK

INT8U temperature = 0;  	    
INT8U humidity = 0; 
extern u16 CO2_value;

extern WIFI_Data  Wifi_Data_Buf;


/********************************************
����        ��WifiRevTask  wifi���ݽ�������
�������ȼ�  ��8
�����ջ��С��128
�����ջ    ��wifiRevTaskStk[WIFI_REV_STK_SIZE]
*********************************************/
#define WIFI_REV_TASK_PRIO    8
#define WIFI_REV_STK_SIZE  	  128
OS_STK wifiRevTaskStk[WIFI_REV_STK_SIZE];
void WifiRevTask(void *pdata);


/********************************************
����        ��WifiTrmTask   wifi���ݷ�������
�������ȼ�  ��9
�����ջ��С��256
�����ջ    ��wifiTrmTaskStk[WIFI_TRM_STK_SIZE]
*********************************************/
#define WIFI_TRM_TASK_PRIO        9
#define WIFI_TRM_STK_SIZE  	      256            //!128 �Ͳ���
OS_STK wifiTrmTaskStk[WIFI_TRM_STK_SIZE];
void WifiTrmTask(void *pdata);


/********************************************
����        :UartTask     UART1�������
�������ȼ�  :10
�����ջ��С:128
�����ջ    :uartTaskStk[KEYO_STK_SIZE]
*********************************************/
#define UART_TASK_PRIO       10  	  
#define UART_STK_SIZE  		 128
OS_STK uartTaskStk[UART_STK_SIZE];
void UartTask(void *pdata);


/********************************************
����        ��ControlTask
�������ȼ�  ��11
�����ջ��С��128
�����ջ    ��ControlTaskStk[Control_STK_SIZE]
*********************************************/
#define Control_TASK_PRIO    11
#define Control_STK_SIZE     128
OS_STK ControlTaskStk[Control_STK_SIZE];
void ControlTask(void *pdata);

/********************************************
����        ��LcdShowTask  LCD��ʾ������
�������ȼ�  ��14
�����ջ��С��128
�����ջ    ��lcdShowTaskStk[LCD_SHOW_STK_SIZE]
*********************************************/
#define LCD_SHOW_TASK_PRIO   14    	  
#define LCD_SHOW_STK_SIZE    128  		    	
OS_STK lcdShowTaskStk[LCD_SHOW_STK_SIZE];
void LcdShowTask(void *pdata);


/********************************************
����        ��TP_Task    TP����������
�������ȼ�  ��15
�����ջ��С��256
�����ջ    ��tpTaskStk[TP_STK_SIZE];
*********************************************/
#define TP_TASK_PRIO         15
#define TP_STK_SIZE  	     128
OS_STK tpTaskStk[TP_STK_SIZE];
void TP_Task(void *pdata);

/********************************************
����        ��START ����������������
�������ȼ�  ��16
�����ջ��С��256
�����ջ    ��startTaskSTK[START_STK_SIZE]
*********************************************/
#define START_TASK_PRIO      16
#define START_STK_SIZE  	 128
OS_STK startTaskSTK[START_STK_SIZE];
void StartTask(void *pdata);


/*************************************************************
*Function Name  : IwdgTmrCallBack
*Auther         : ����
*Vertion        : v1.0
*Date           : 2016-12-06
*Description    : ��ʱ���ص�����������ι����С�Ƶ���˸
*Input          : 1. void *ptmr
                  2. void *parg
				  û�д��ݵĲ��������������
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
	
	IwdgFeed();  //ι��
	
}


u8 err = 0;               //���󷵻���


int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
	
	delay_init(168);      //��ʼ����ʱ����
	LedInit();		      //��ʼ��LED�˿� 
	ClassroomLedInit();   //��ʼ�����ҵ���յLED���ƽӿ�
	UART1_Init(115200);   //��ʼ������1
	AdcInit();            //��ʼ��ADC�ӿ�
	LCD_Init();	          //��ʼ��LCD	
	KeyInit();   	      //��ʼ������
	InfradedInit();       //��ʼ�����⴫����
	DcMotorInit();        //��ʼ��ֱ�����
	
	OSInit();             //��ʼ��uC/OS
	OSTaskCreate(StartTask, (void*)0, (OS_STK *)&startTaskSTK[START_STK_SIZE-1],START_TASK_PRIO );//������ʼ����
	OSStart();	
	
	return 0;
}

/*************************************************************
*Function Name  : StartTask
*Auther         : ����
*Vertion        : v1.0
*Date           : 2016-12-05
*Description    : ��ʼ���� ���������ڽ�������������
*Input          ��
*Output         ��
*Return         ��
*************************************************************/
void StartTask(void *pdata)
{
	OS_CPU_SR cpu_sr = 0;
	
	pdata = pdata; 
	
	semWifi = OSSemCreate(0);	       //�����ź���	���յ�ESP8266��������	 
	semWifiData = OSSemCreate(0);      //�����ź���	���յ�����������
	wifiConnectState = ESP8266_Init(); //��ʼ��ESP8266����ӿڣ����������״̬
	
	flagTP = OSFlagCreate(0, &err);    //�����¼���־��
	
	OSStatInit();					   //��ʼ��ͳ������.�������ʱ1��������
	
	OS_ENTER_CRITICAL();		       //�����ٽ���(�޷����жϴ��)    
	
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
	
	IwdgInit(4,1500);                 //IWDG�ĳ�ʱ����Ϊ3S
	OSTaskSuspend(START_TASK_PRIO);	  //������ʼ����.
	OS_EXIT_CRITICAL();				  //�˳��ٽ���(���Ա��жϴ��)
} 


/*************************************************************
*Function Name  : WifiRevTask
*Auther         : ����
*Vertion        : v1.0
*Date           : 2017-04-20
*Description    : wifi���ݽ�������
*Input          ��
*Output         ��
*Return         ��
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
*Auther         : ����
*Vertion        : v1.0
*Date           : 2017-04-20
*Description    : wifi���ݷ�������
*Input          ��
*Output         ��
*Return         ��
*************************************************************/
void WifiTrmTask(void *pdata)
{
	pdata =  pdata;
		
	u8 errWifiTrmTask;
	char HTTP_Buf[100];        //HTTP���Ļ�����
	char charBuff[8] = {0};    //wifi�����ϴ�������
	u8 len;
	ESP8266_Con_Udp(wifiConnectState);
	
	while(1)
	{
		OSSemPend(semWifiData, OS_TICKS_PER_SEC*10, &errWifiTrmTask);
		memset(&Wifi_Data_Buf, 0, sizeof(Wifi_Data_Buf));//��սṹ��Wifi_Data_Buf
		
		/******������ʪ�ȡ�CO2Ũ��******/
		sprintf(charBuff, "%d", CO2_value);
		len = HTTP_PostPkt(HTTP_Buf, API_KEY, DEV_ID, "CO2", charBuff);
		USART3_Send_Data((u8 *)HTTP_Buf, len);
		memset(HTTP_Buf, 0, sizeof(HTTP_Buf));           //���HTTP_Buf
		sprintf(charBuff, "%d", temperature);
		len = HTTP_PostPkt(HTTP_Buf, API_KEY, DEV_ID, "temp", charBuff);
		USART3_Send_Data((u8 *)HTTP_Buf, len);
		memset(HTTP_Buf, 0, sizeof(HTTP_Buf));           //���HTTP_Buf
		sprintf(charBuff, "%d", humidity);
		len = HTTP_PostPkt(HTTP_Buf, API_KEY, DEV_ID, "humi", charBuff);
		USART3_Send_Data((u8 *)HTTP_Buf, len);
		memset(HTTP_Buf, 0, sizeof(HTTP_Buf));           //���HTTP_Buf
		
		/******���ͷ��ȹ���״̬��ת��******/
		if (fanSpeed == 0)
		{
			len = HTTP_PostPkt(HTTP_Buf, API_KEY, DEV_ID, "fanState", (char*)"0");
		}
		else
		{
			len = HTTP_PostPkt(HTTP_Buf, API_KEY, DEV_ID, "fanState", (char*)"1");
		}
		 USART3_Send_Data((u8 *)HTTP_Buf, len);
		 memset(HTTP_Buf, 0, sizeof(HTTP_Buf));           //���HTTP_Buf
		
		sprintf(charBuff, "%d", fanSpeed);
		len = HTTP_PostPkt(HTTP_Buf, API_KEY, DEV_ID, "fanSpeed", charBuff);
		USART3_Send_Data((u8 *)HTTP_Buf, len);
		memset(HTTP_Buf, 0, sizeof(HTTP_Buf));           //���HTTP_Buf
		
		/******���Ϳ���յ��******/
		sprintf(charBuff, "%d", ledNum);
		len = HTTP_PostPkt(HTTP_Buf, API_KEY, DEV_ID, "ledNum", charBuff);
		USART3_Send_Data((u8 *)HTTP_Buf, len);
		memset(HTTP_Buf, 0, sizeof(HTTP_Buf));           //���HTTP_Buf
		
		/******���ͽ�������******/
		sprintf(charBuff, "%d", peopelNum);
		len = HTTP_PostPkt(HTTP_Buf, API_KEY, DEV_ID, "peopelNum", charBuff);
		USART3_Send_Data((u8 *)HTTP_Buf, len);
		memset(HTTP_Buf, 0, sizeof(HTTP_Buf));           //���HTTP_Buf
		
		
		OSTimeDly(OS_TICKS_PER_SEC*3);
	}
}


/*************************************************************
*Function Name  : UartTask
*Auther         : ����
*Vertion        : v1.0
*Date           : 2017-04-16
*Description    : Uart1��ʾ����
*Input          ��
*Output         ��
*Return         ��
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
		printf("WIFI����״̬ %d\r\n", wifiConnectState);
		printf("OSTaskCtr = %d\r\n", OSTaskCtr);
		#endif

		OSTimeDly(OS_TICKS_PER_SEC*3);
	}
}

/*************************************************************
*Function Name  : ControlTask
*Auther         : ����
*Vertion        : v1.0
*Date           : 2017-10-09
*Description    : ��������
                  ���ƽ�����յ�Ƶ������Զ�ģʽ���ֶ�ģʽ��
*Input          ��
*Output         ��
*Return         ��
*************************************************************/
void ControlTask(void *pdata)
{
	pdata = pdata;
	INT8U i = 0;
	
	while (1)
	{
		ledNum = 0;
		/******�������ǰ�Ŀ�������******/
		for (i=0; i<4; i++)
		{
			if ( (ledFlag & (0x01 << i)) == 0 )
			{
				ledNum++;
			}
		}
		/******�Զ�����ģʽ******/
		if (controlState == 0)
		{
			/*�������������ǰ�ƹ⸺������,�򿪵�*/
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
			/*����������㣬����������Ƿ���Ҫ�ص�*/
			else                      
			{
				for (i=0; i<4; i++)
				{
					/*������ƿ��ţ�����û��ʱ����ص�*/
					if ( ((ledFlag & (0x01 << i)) == 0) && (InfradedRead(i) == 0) )
					{
						ledFlag |= (0x01 << i);//��־λ��ΪϨ��״̬
						if (ledNum != 0)
						{
							ledNum--;          //����������1					
						}
					}
				}
			}
		}
		
		/******����С�Ƶ�����******/
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
		
		/******���Ʒ��ȵ�������ֹͣ��ת��******/
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
*Auther         : ����
*Vertion        : v1.0
*Date           : 2017-01-14
*Description    : ����ʾ������ʾ LED2��״̬�Ͱ�����״̬��
*Input          ��
*Output         ��
*Return         ��
*************************************************************/
void LcdShowTask(void *pdata)
{
	pdata =  pdata;
	
	char charBuff[8] = {0};    //������ʾ������
	
	INT8U timeNum = 0;    //ʱ�������־
	DHT11_Init();
	
	OSTimeDly(OS_TICKS_PER_SEC);
	while(1)
	{	
		/******ÿ��1S��ȡ��������ֵ ********/
		if (timeNum == 5)
		{
			timeNum = 0;  //ʱ���־λ��λ
			 
			/*��ȡ��ʪ��ֵ*/
			DHT11_Read_Data(&temperature, &humidity); 
			
			/*��ȡCO2��ֵ*/
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
		
		/******CPUʹ���������********/
		LCD_ShowString(56, 0, 104, 16, 16, (u8*)"CPU Usage:  %");
		LCD_ShowNum(136, 0, OSCPUUsage, 2, 16);	
		
	    /******WIFI��TCP�����������ʾ����********/
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
		
		/******����ת��********/
		sprintf(charBuff, "FS:%3d", fanSpeed);
		LCD_ShowString(190, 30, 48, 16, 16, (u8*)charBuff);
		
		/******��ʪ�Ƚ���********/
		LCD_ShowString(10, 100, 72, 16, 16, (u8*)"Temp:   C");	
        LCD_Draw_Circle(71, 104, 2);		
 	    LCD_ShowString(90, 100, 64, 16, 16, (u8*)"Humi:  %");
		LCD_ShowNum(50, 100, temperature, 2, 16);			   		   
		LCD_ShowNum(130, 100, humidity, 2, 16);		

		/******CO2��ʾ����********/
		sprintf(charBuff, "CO2:%5d", CO2_value);
		LCD_ShowString(165, 100, 88, 16, 16, (u8*)charBuff);		
		
		/******�Զ����ֶ����ƽ���********/
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
		
		/******���ҵƱ�Ž���********/
		LCD_ShowString(30,  170, 1, 24, 24, (u8*)"1");
		LCD_ShowString(80,  170, 1, 24, 24, (u8*)"2");
		LCD_ShowString(130, 170, 1, 24, 24, (u8*)"3");
		LCD_ShowString(180, 170, 1, 24, 24, (u8*)"4");
		
		/******�ƹ�״̬�����ƽ���********/
		if ((ledFlag & 0x01) != 0)  //1��
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
		if ((ledFlag & 0x02) != 0)  //2��
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
		if ((ledFlag & 0x04) != 0)  //3��
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
		if ((ledFlag & 0x08) != 0)  //4��
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
		
		/******������⹤�������ʾ����********/
		if (InfradedRead(0) == 0) Show_Circle(30, 283, 10, WHITE);
		else                      Show_Circle(30, 283, 10, BLUE);
		if (InfradedRead(1) == 0) Show_Circle(80, 283, 10, WHITE);
		else                      Show_Circle(80, 283, 10, BLUE);
		if (InfradedRead(2) == 0) Show_Circle(130, 283, 10, WHITE);
		else                      Show_Circle(130, 283, 10, BLUE);
		if (InfradedRead(3) == 0) Show_Circle(180, 283, 10, WHITE);
		else                      Show_Circle(180, 283, 10, BLUE);

		/******����������ʾ����********/
		sprintf(charBuff, "Led:%d", ledNum);
		LCD_ShowString(10, 300, 56, 16, 16, (u8*)charBuff);
		
		/******����������ʾ����********/
		sprintf(charBuff, "peopel:%3d", peopelNum);
		LCD_ShowString(150, 300, 81, 16, 16, (u8*)charBuff);
		
		OSTimeDly(100);  //��ʱ100��Ticks = 200ms 
	}
}


/*************************************************************
*Function Name  : TP_Task
*Auther         : ����
*Vertion        : v1.0
*Date           : 2017-04-20
*Description    : TP����������
*Input          ��
*Output         ��
*Return         ��
*************************************************************/
void TP_Task(void *pdata)
{
	pdata =  pdata;
	u8 errTP_Task;
	
	/*********************��������ʼ��*********************/
    tp_dev.init();		  //��������ʼ��
	POINT_COLOR=RED;      //��������Ϊ��ɫ 
	if (tp_dev.touchtype != 0xFF)
	{
		LCD_ShowString(30, 130 ,200, 16, 16, (u8*)"Press KEY0 to Adjust");
	}
	Load_Drow_Dialog();   //�����Ļ�������Ͻ���ʾ"CLR"
	/******************************************************/
	
	while(1)
	{
		OSFlagPend(flagTP, 0x01, OS_FLAG_WAIT_SET_ALL + OS_FLAG_CONSUME, 0, &errTP_Task);
		tp_dev.scan(0);
		if (tp_dev.x[0]>(lcddev.width-24) && tp_dev.y[0]<16)
		{
			Load_Drow_Dialog();     //���
			OSFlagPost(flagTP, 0x04, OS_FLAG_SET, &errTP_Task);
		}
		else if (tp_dev.x[0]>20 && tp_dev.x[0]<90 && tp_dev.y[0]>120 && tp_dev.y[0]<150)
		{
			controlState = 0;      //���"Auto"   �Զ�ģʽ
		}
		else if (tp_dev.x[0]>120 && tp_dev.x[0]<202 && tp_dev.y[0]>120 && tp_dev.y[0]<150)
		{
			controlState = 1;      //���"Manual" �ֶ�ģʽ
		}
		else if (tp_dev.x[0]>13 && tp_dev.x[0]<47 && tp_dev.y[0]>200 && tp_dev.y[0]<222)
		{
			if (controlState == 1) ledFlag &= 0x0E;//��һյ��ON
		}
		else if (tp_dev.x[0]>13 && tp_dev.x[0]<47 && tp_dev.y[0]>240 && tp_dev.y[0]<262)
		{
			if (controlState == 1) ledFlag |= 0x01;//��һյ��OFF
		}
		else if (tp_dev.x[0]>63 && tp_dev.x[0]<97 && tp_dev.y[0]>200 && tp_dev.y[0]<222)
		{
			if (controlState == 1) ledFlag &= 0x0D;//�ڶ�յ��ON
		}
		else if (tp_dev.x[0]>63 && tp_dev.x[0]<97 && tp_dev.y[0]>240 && tp_dev.y[0]<262)
		{
			if (controlState == 1) ledFlag |= 0x02;//�ڶ�յ��OFF
		}
		else if (tp_dev.x[0]>113 && tp_dev.x[0]<147 && tp_dev.y[0]>200 && tp_dev.y[0]<222)
		{
			if (controlState == 1) ledFlag &= 0x0B;//����յ��ON
		}
		else if (tp_dev.x[0]>113 && tp_dev.x[0]<147 && tp_dev.y[0]>240 && tp_dev.y[0]<262)
		{
			if (controlState == 1) ledFlag |= 0x04;//����յ��OFF
		}
		else if (tp_dev.x[0]>163 && tp_dev.x[0]<197 && tp_dev.y[0]>200 && tp_dev.y[0]<222)
		{
			if (controlState == 1) ledFlag &= 0x07;//����յ��ON
		}
		else if (tp_dev.x[0]>163 && tp_dev.x[0]<197 && tp_dev.y[0]>240 && tp_dev.y[0]<262)
		{
			if (controlState == 1) ledFlag |= 0x08;//����յ��OFF
		}
		else
		{
			TP_Draw_Big_Point(tp_dev.x[0], tp_dev.y[0], RED);
		}
	}
}
/*******************************����ݸ���*******************************/
#if 0


#endif

