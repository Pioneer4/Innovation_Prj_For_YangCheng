/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : spi.c
*Auther         : 张沁
*Version        : v1.0
*Date           : 2016-09-22
*Description    : 该文件包含了有关SPI1的初始化和中断使能
*Function List  : 
*************************************************************/
#include "myinclude.h"

//u8 SPI1_Data_Rev = 0x00;

/*************************************************************
*Function Name  : SPI1_Init
*Auther         : 张沁
*Version        : v1.0
*Date           : 2016-09-22
*Description    : 初始化SPI1
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void SPI1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;  
	SPI_InitTypeDef SPI_InitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);             //使能SPI1时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);  	 		 //使能GPIOB的时钟
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;                        //复用功能
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;                      //推挽输出
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5; //PB3、4、5复用输出
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;  					     //上拉
	GPIO_InitStruct.GPIO_Speed = GPIO_High_Speed;                    //速度100MHz
	GPIO_Init(GPIOB, &GPIO_InitStruct);  //初始化GPIOB
	
	
	/********引脚复用***************/
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1);          //PB4  SPI1_MISO
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);  		 //PB5  SPI1_MOSI
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);          //PB3  SPI1_SCK
	
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;//波特率预分频值为256
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_2Edge;                        //串行时钟输入的的第二个跳变沿（这里是上升）数据被采样
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_High;                         //串行时钟输入高电平有效
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;	                 //设置SPI的数据大小：发送接收8为帧结构
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //SPI设置为双线双向全双工
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;  				 //数据从高位开始传输
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;  					 //主SPI
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft; 							 //内部NSS信号由SSI位控制
	SPI_Init(SPI1, &SPI_InitStruct);
	
	SPI_Cmd(SPI1, ENABLE);  //使能SPI外设
	
	SPI1_ReadWriteByte(0xff);//启动传输	
}


/*************************************************************
*Function Name  : SPI1_ReadWriteByte
*Auther         : 张沁
*Version        : v1.0
*Date           : 2016-09-23
*Description    : SPI1 读写一个字节
*Input          ：TxData  要写入的字节
*Output         ：
*Return         ：读到的字节
*************************************************************/
u8 SPI1_ReadWriteByte(u8 TxData)
{
	while(SPI_GetFlagStatus(SPI1, SPI_FLAG_TXE) == RESET);  //等待发送区为空
	SPI_SendData(SPI1, (u16)TxData);                        //通过SPI1发送一个Byte数据
	
	while (SPI_GetFlagStatus(SPI1, SPI_FLAG_RXNE) == RESET);//等待接收缓冲区非空（接收完1Byte）
	
	return SPI_ReceiveData(SPI1);                           //通过SPI返回接收的数据
}


/*************************************************************
*Function Name  : SPI1_SetSpeed
*Auther         : 张沁
*Version        : v1.0
*Date           : 2016-09-23
*Description    : SPI1速度设置函数，SPI速度=fAPB2/分频系数
*Input          ：SPI_BaudRatePrescaler  分频系数
*Output         ：
*Return         ：
*************************************************************/
void SPI1_SetSpeed(u8 SPI_BaudRatePrescaler)
{
	assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));//判断有效性
	SPI1->CR1 &= 0XFFC7;                 //位3-5清零，用来设置波特率
	SPI1->CR1 |= SPI_BaudRatePrescaler;  //设置SPI1的分频系数
	SPI_Cmd(SPI1,ENABLE);	             //使能SPI1
}
