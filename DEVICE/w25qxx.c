/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : w25qxx.c
*Auther         : 张沁
*Version        : v1.0
*Date           : 2016-09-23
*Description    : W25QXX 驱动代码
*Function List  : 
*************************************************************/
#include "w25qxx.h"

u16 W25QXX_TYPE = W25Q16;  //默认为是W25Q16

/*************************************************************
*Function Name  : W25QXX_Init
*Auther         : 张沁
*Version        : v1.0
*Date           : 2016-09-23
*Description    : W25Q16 初始化
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void W25QXX_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	W25QXX_CS = 1;                         //CS端置高电平  FLASH不选中
	
	SPI1_Init();                           //初始化SPI1接口
    SPI1_SetSpeed(SPI_BaudRatePrescaler_2);//主频168M PB2=84M  SPI=PB2/2=42M时钟,高速模式 
	W25QXX_TYPE = W25QXX_ReadID();         //读取FLASH ID.
}


/*************************************************************
*Function Name  : W25QXX_ReadID
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2016-09-23
*Description    : 读取W25QXX芯片ID
*Input          ：
*Output         ：
*Return         ：芯片的ID
*************************************************************/
//0XEF13,表示芯片型号为W25Q80  
//0XEF14,表示芯片型号为W25Q16    
//0XEF15,表示芯片型号为W25Q32  
//0XEF16,表示芯片型号为W25Q64 
//0XEF17,表示芯片型号为W25Q128 
u16  W25QXX_ReadID(void)
{
	u16 Temp = 0;
	
	W25QXX_CS = 0;
	SPI1_ReadWriteByte(W25X_ManufactDeviceID);  //发送读取ID（制造设备）命令
	SPI1_ReadWriteByte(0x00);
	SPI1_ReadWriteByte(0x00);
	SPI1_ReadWriteByte(0x00); 	
	Temp |= (SPI1_ReadWriteByte(0xFF) << 8);
	Temp |= SPI1_ReadWriteByte(0xFF);
	W25QXX_CS = 1;
	
	return Temp;
}


/*************************************************************
*Function Name  : W25QXX_ReadSR
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2016-09-23
*Description    : 读取W25QXX的状态寄存器
*Input          ：
*Output         ：
*Return         ：状态寄存器数据
*************************************************************/
//读取W25QXX的状态寄存器
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:默认0,状态寄存器保护位,配合WP使用
//TB,BP2,BP1,BP0:FLASH区域写保护设置
//WEL:写使能锁定
//BUSY:忙标记位(1,忙;0,空闲)
//默认:0x00
u8	 W25QXX_ReadSR(void)
{
	u8 byte = 0;
	
	W25QXX_CS = 0;                          //片选
	SPI1_ReadWriteByte(W25X_ReadStatusReg); //发送读取状态寄存器命令
	byte = SPI1_ReadWriteByte(0Xff);	    //读取一个字节
	W25QXX_CS = 1;							//取消片选
	
	return byte;
}


/*************************************************************
*Function Name  : W25QXX_Write_SR
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2016-09-23
*Description    : 写入25QXX的状态寄存器
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
//写W25QXX状态寄存器
//只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写!!!
void W25QXX_Write_SR(u8 sr)
{
	W25QXX_CS=0;                             //片选  
	SPI1_ReadWriteByte(W25X_WriteStatusReg); //发送写取状态寄存器命令
	SPI1_ReadWriteByte(sr);                  //写入一个字节
	W25QXX_CS = 1; 				             //取消片选
}


/*************************************************************
*Function Name  : W25QXX_Write_Enable
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2016-09-23
*Description    : W25QXX写使能，将WEL置位 
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void W25QXX_Write_Enable(void)
{
	W25QXX_CS = 0;                         //片选
	SPI1_ReadWriteByte(W25X_WriteEnable);  //发送写使能指令
	W25QXX_CS = 1;                         //取消片选
}


/*************************************************************
*Function Name  :  W25QXX_Write_Disable
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2016-09-23
*Description    : W25QXX写禁止
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void W25QXX_Write_Disable(void)
{
	W25QXX_CS = 0;                         //片选
	SPI1_ReadWriteByte(W25X_WriteDisable); //发送写禁止指令
	W25QXX_CS = 1;                         //取消片选
}


/*************************************************************
*Function Name  : W25QXX_Write_NoCheck
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2016-09-23
*Description    : 无检验写FLASH
                  1.必须确定所写的地址范围内的全部数据为0xff，否则在非0xff下写入的数据将失败!
				  2.具有自动换页的功能
				  3.在指定地址开始写入指定长度的数据，但是要确保地址不越界！
*Input          ：1. pBuffer        数据存储区
				  2. WriteAddr      开始写入的地址（24bit）
				  3. NumByteToWrite 要写入的字节数（最大65535）
*Output         ：
*Return         ：
*************************************************************/
void W25QXX_Write_NoCheck(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)
{
	u16 pageremain;	
	pageremain = 256 - WriteAddr%256;  //单页剩余的字节数
	
	if(NumByteToWrite <= pageremain)   //不大于256个字节
	{
		pageremain = NumByteToWrite;
	}
	while(1)
	{
		W25QXX_Write_Page(pBuffer, WriteAddr, pageremain);
		
		/********写入结束*********/
		if(NumByteToWrite == pageremain)
			break;  
		
		/******* NumByteToWrite > pageremain **********/
		else
		{
			pBuffer += pageremain;  //数据位移动
			WriteAddr += pageremain;  //地址位移动
			NumByteToWrite -= pageremain;  //减去已经写入的字节数
			
			if(NumByteToWrite > 256)
				pageremain = 256;  //一次写入256个字节
			else
				pageremain = NumByteToWrite;  //不够256个字节
		}
	}
}


/*************************************************************
*Function Name  : W25QXX_Write_Page
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2016-09-23
*Description    : 在一页（256Byte）内写入不大于256个字节的数据
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void W25QXX_Write_Page(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)
{
	u16 i ;
	W25QXX_Write_Enable();                //发送写命令
	W25QXX_CS = 0;                        //片选
	SPI1_ReadWriteByte(W25X_PageProgram); //SET WEL(The WEL bit must be set prior to every Page Program)
	
	/********发送24bit地址***********/
	SPI1_ReadWriteByte((u8)((WriteAddr)>>16));
	SPI1_ReadWriteByte((u8)((WriteAddr)>>8));
	SPI1_ReadWriteByte((u8)WriteAddr);
	
	/******循环写数********/
	for(i = 0; i < NumByteToWrite; i++)
	{
		SPI1_ReadWriteByte(pBuffer[i]);
	}
	
	W25QXX_CS = 1;                       //取消片选
	W25QXX_Wait_Busy();                  //等待写入结束
}


/*************************************************************
*Function Name  : W25QXX_Read
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2016-09-23
*Description    : 读取FLASH
*Input          ：1. pBuffer       数据存储区
				  2. ReadAddr      读取地址
				  3. NumByteToRead 要读取的字节数(最大65535)
*Output         ：
*Return         ：
*************************************************************/
void W25QXX_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead)  
{
	u16 i; 
	W25QXX_CS=0;                        //片选
	SPI1_ReadWriteByte(W25X_ReadData);  //发送读取指令

	/*******发送24bit地址*******/
	SPI1_ReadWriteByte((u8)((ReadAddr)>>16)); 
	SPI1_ReadWriteByte((u8)((ReadAddr)>>8));
	SPI1_ReadWriteByte((u8)ReadAddr);
	
	/*****循环读数 ******/
	for(i = 0; i < NumByteToRead; i++)
	{
		 pBuffer[i] = SPI1_ReadWriteByte(0XFF);   	 
	}
    
	W25QXX_CS = 1;
}


/*************************************************************
*Function Name  : W25QXX_Write
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2016-09-23
*Description    : 写FLASH
				  1. 在指定地址开始写入指定长度的数据
				  2. 该函数带擦除操作!
*Input          ：1. pBuffer       数据存储区
				  2. WriteAddr     开始写入的地址(24bit)
				  3. NumByteToRead 要读取的字节数(最大65535)
*Output         ：
*Return         ：
*************************************************************/
u8 W25QXX_BUFFER[4096];	 
void W25QXX_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)
{
	u32 secpos;
	u16 secoff;
	u16 secremain;
	u16 i;
	
	u8 * W25QXX_BUF;
	W25QXX_BUF=W25QXX_BUFFER;
	
	secpos = WriteAddr/4096;                         //扇区地址
	secoff = WriteAddr%4096;                         //在扇区内的偏移
	secremain = 4096 - secoff;                       //扇区剩余空间大小
	
	if(NumByteToWrite <= secremain) 
		secremain = NumByteToWrite;                  //不大于4096个字节
	
	while(1)
	{
		W25QXX_Read(W25QXX_BUF, secpos * 4096, 4096);//读取整个扇区的内容
		for(i = 0; i < secremain; i++)
		{
			if(W25QXX_BUF[secoff + i] != 0xff)
				break;                               //需要擦擦
		}
		if(i < secremain)                            //需要擦除
		{
			W25QXX_Erase_Sector(secpos);             //擦除扇区内容
			for(i = 0; i < secremain; i++)           //复制
			{
				W25QXX_BUF[i+secoff]=pBuffer[i];
			}
			W25QXX_Write_NoCheck(W25QXX_BUF,secpos*4096,4096);//写入整个扇区 
		}
		else
			W25QXX_Write_NoCheck(pBuffer,WriteAddr,secremain);//写已经擦除了的,直接写入扇区剩余区间.
		
		if(NumByteToWrite  == secremain)
			break;                         //写入结束
		else                               //写入未结束
		{
			secpos++;                      //扇区地址增1
			secoff=0;                      //偏移位置为0
			
			pBuffer += secremain;          //指针偏移
			WriteAddr += secremain;        //写地址偏移
			NumByteToWrite -= secremain;   //字节数递减
			if(NumByteToWrite>4096) 
				secremain = 4096;	       //下一个扇区还是写不完
			else 
				secremain = NumByteToWrite;//下一个扇区可以写完了
		}
	};
}


/*************************************************************
*Function Name  : W25QXX_Erase_Sector
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2016-09-24
*Description    : 擦除一个扇区  擦除一个扇区的最少时间:150ms  
*Input          ：1. Dst_Addr  扇区地址 根据实际容量设置
*Output         ：
*Return         ：
*************************************************************/
void W25QXX_Erase_Sector(u32 Dst_Addr)	
{
    //监视falsh擦除情况,测试用   
 	printf("fe:%x\r\n",Dst_Addr);
	
	Dst_Addr *= 4096;
	
	W25QXX_Write_Enable(); 
	W25QXX_Wait_Busy();  
	
	W25QXX_CS = 0;
	SPI1_ReadWriteByte(W25X_SectorErase);//发送扇区擦除指令
	
	/*******发送24bit地址*******/
	SPI1_ReadWriteByte((u8)(Dst_Addr >> 16));
	SPI1_ReadWriteByte((u8)(Dst_Addr >> 8));
	SPI1_ReadWriteByte((u8)Dst_Addr);
	
	W25QXX_CS = 1;
	W25QXX_Wait_Busy();                  //等待擦除完成
}


/*************************************************************
*Function Name  : W25QXX_Erase_Chip
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2016-09-24
*Description    : 擦除整个芯片   等待时间超长.. 
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void W25QXX_Erase_Chip(void)
{
	W25QXX_Write_Enable(); 
	W25QXX_Wait_Busy(); 
	
	W25QXX_CS = 0;                      //片选
	SPI1_ReadWriteByte(W25X_ChipErase); //发送片擦除命令  
	W25QXX_CS=1;                        //取消片选     	      
	
	W25QXX_Wait_Busy();                 //等待芯片擦除结束
}


/*************************************************************
*Function Name  : W25QXX_Wait_Busy
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2016-09-23
*Description    : 等待空闲
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void W25QXX_Wait_Busy(void)
{
	while((W25QXX_ReadSR()&0x01)==0x01);  // 等待BUSY位清空
}


/*************************************************************
*Function Name  : W25QXX_PowerDown
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2016-09-24
*Description    : 进入掉电模式
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void W25QXX_PowerDown(void)   
{ 
  	W25QXX_CS=0;                        //使能器件   
    SPI1_ReadWriteByte(W25X_PowerDown); //发送掉电命令  
	W25QXX_CS=1;                        //取消片选   
	
    delay_us(3);                        //等待TPD  
}   


/*************************************************************
*Function Name  : W25QXX_WAKEUP
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2016-09-24
*Description    : 唤醒FLASH
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void W25QXX_WAKEUP(void)   
{  
  	W25QXX_CS=0;                              //使能器件   
    SPI1_ReadWriteByte(W25X_ReleasePowerDown);//发送唤醒命令  
	W25QXX_CS=1;                              //取消片选     
	
    delay_us(3);                              //等待TRES1
}   
