/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : dht11.c
*Auther         : 张沁
*Version        : v1.0
*Date           : 2016-05-03
*Description    : 该文件包含了dht11的驱动代码
*Function List  : 
*************************************************************/

#include "myinclude.h"

/*************************************************************
*Function Name  : DHT11_Init
*Auther         : 张沁
*Version        : v1.0
*Date           : 2016-05-03
*Description    : 初始化DHT11的DAT口(PE2) 
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void DHT11_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
}


/*************************************************************
*Function Name  : DHT11_Rst
*Auther         : 张沁
*Version        : v1.0
*Date           : 2016-05-03
*Description    : 复位DHT11
*Input          ：
*Output         ：
*Return         ：1,存在    0,不存在
*************************************************************/
void DHT11_Rst(void)
{
	DHT11_IO_OUT(); 
	DHT11_DQ_OUT = 0;
    delay_ms(20);	  //主机至少拉低18ms
	DHT11_DQ_OUT = 1;
	delay_us(30);	  //主机至少拉高20~40us
}


/*************************************************************
*Function Name  : DHT11_Check
*Auther         : 张沁
*Version        : v1.0
*Date           : 2016-05-03
*Description    : 等待DHT11的回应 若没有回应则表示不存在
*Input          ：
*Output         ：
*Return         ：1,存在    0,不存在
*************************************************************/
u8 DHT11_Check(void) 	
{
	u8 retry=0;
	
	DHT11_IO_IN();
	while (DHT11_DQ_IN && retry<60) //DHT11会拉低40~50us
	{
		retry++;
		delay_us(1);
	}
	if (retry == 60)
	{
		return 0;                   //等待应答失败
	}
	else
	{
		retry = 0;
	}
	
	while (!DHT11_DQ_IN && retry<60)//DHT11会拉高40~50us
	{
		retry++;
		delay_us(1);
	}
	if (retry == 60)
	{
		return 0;                   //等待应答失败
	}
	
	return 1;                       //等待应答成功
}

/*************************************************************
*Function Name  : DHT11_Read_Bit
*Auther         : 张沁
*Version        : v1.0
*Date           : 2016-05-03
*Description    : 从DHT11读取一个位
*Input          ：
*Output         ：
*Return         ：1,读到高电平    0，读到低电平
*************************************************************/
u8 DHT11_Read_Bit(void)
{
	u8 retry = 0;
	
	while (DHT11_DQ_IN && retry<100) //等待变为低电平
	{
		retry++;
		delay_us(1);
	}
	
	retry=0;
	while (!DHT11_DQ_IN && retry<100)//等待变高电平
	{
		retry++;
		delay_us(1);
	}
	
	/***表示数字0的信号,高电平持续26~28us.数字1的信号,高电平持续116~118us***/
	delay_us(40);                  
	if (DHT11_DQ_IN == 1)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*************************************************************
*Function Name  : DHT11_Read_Bit
*Auther         : 张沁
*Version        : v1.0
*Date           : 2016-05-03
*Description    : 从DHT11读取一个字节数据
*Input          ：
*Output         ：
*Return         ：读取的一字节数据
*************************************************************/
u8 DHT11_Read_Byte(void)
{
	u8 i = 0;
	u8 dat = 0;
	for (i=0; i<8; i++) 
	{
   		dat <<= 1; 
	    dat |= DHT11_Read_Bit();
    }
	
	return dat;
}


/*************************************************************
*Function Name  : DHT11_Read_Data
*Auther         : 张沁
*Version        : v1.0
*Date           : 2016-05-03
*Description    : 从DHT11读取一个字节数据
*Input          ：1. *temp  温度数据保存区
                  2. *humi  湿度数据保护区
*Output         ：
*Return         ：1,数据获取成功   0,数据获取失败
*************************************************************/
u8 DHT11_Read_Data(u8 *temp, u8 *humi)
{
	u8 buf[5];
	u8 i;
	
	DHT11_Rst();
	if (DHT11_Check()==1)
	{
		for(i=0; i<5; i++)//读取40位数据
		{
			buf[i] = DHT11_Read_Byte();
		}
		if ( (buf[0]+buf[1]+buf[2]+buf[3])==buf[4] )
		{
			*humi=buf[0];
			*temp=buf[2];
		}
	}
	else 
	{
		return 0;
	}
	
	return 1;
}
