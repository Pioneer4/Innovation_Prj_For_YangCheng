/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : savedata.c
*Auther         : 张沁
*Version        : v1.0
*Date           : 2016-05-03
*Description    : 该文件包含了传单机保存的驱动代码
*Function List  : 
*************************************************************/

#include "myinclude.h"
#include "includes.h"

#if 0
extern INT16U firstLevelNum; //第一层已经发出的传单数量
extern INT16U secondLevelNum;//第二层已经发出的传单数量

/*************************************************************
*Function Name  : PaperNumSaveData
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-10-09
*Description    : 保存传单机中已经发送的传单数量
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
//保存FLASH里面的地址区间基址,占用24个字节(RANGE:PID_SAVE_ADDR_BASE~PID_SAVE_ADDR_BASE+23)
#define  PID_SAVE_ADDR_BASE  20
void PaperNumSaveData(void)
{
	INT8U result_data[4];  //存储缓冲区
	INT16U temp = 0;
	
	/******将firstLevelNum、secondLevelNum换为存储数据******/
	/*firstLevelNum*/
	temp = firstLevelNum;
	result_data[0] =  (u8)(temp >> 8);
	result_data[1] =  (u8)temp;	
	/*secondLevelNum*/
	temp = secondLevelNum;
	result_data[2] =  (u8)(temp >> 8);
	result_data[3] =  (u8)temp;
	
	W25QXX_Write(result_data, PID_SAVE_ADDR_BASE, 4);  //校正结果写入FLASH
}


/*************************************************************
*Function Name  : PaperNumGetData
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-10-09
*Description    : 得到保存在FLASH里面的传单数量值
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void PaperNumGetData(void)
{
	INT8U result_data[4];  //存储缓冲区
	INT16U temp = 0;
	
	W25QXX_Read(result_data, PID_SAVE_ADDR_BASE, 4);  //读取FLASH中的数据
	
	/*得到firstLevelNum*/
	temp = result_data[0];
	temp = (temp << 8) + result_data[1];
	firstLevelNum = temp;
	/*得到secondLevelNum*/
	temp = result_data[2];
	temp = (temp << 8) + result_data[3];
	secondLevelNum = temp;
}
#endif
