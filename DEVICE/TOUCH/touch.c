/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : touch.c
*Auther         : 张沁
*Version        : v1.0
*Date           : 2017-04-30
*Description    : 触摸屏驱动   (支持XPT2046)
*Function List  : 
*History        :
1. Ｄate: 2017-05-01  Auther: 张沁
Modification: 修正了TP_Save_Adjdata和TP_Adj_Info_Show函数
*************************************************************/
#include "myinclude.h"

#if SYSTEM_SUPPORT_OS
#include "includes.h"  //ucos 使用	  
#endif

extern OS_FLAG_GRP *flagTP;

_m_tp_dev tp_dev=
{
	TP_Init,
	TP_Scan,
	TP_Adjust,
	0,
	0, 
	0,
	0,
	0,
	0,	  	 		
	0,
	0,	  	 		
};		

/******默认为touchtype=0的数据******/
u8 CMD_RDX=0XD0;
u8 CMD_RDY=0X90;
 	 			    					   
/*************************************************************
*Function Name  : TP_Write_Byte
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-30
*Description    : 向触摸屏IC写入1byte数据
*Input          ：num  要写入的数据
*Output         ：
*Return         ：
*************************************************************/
void TP_Write_Byte(u8 num)    
{  
	u8 count=0;  
	
	for(count=0;count<8;count++)  
	{ 	  
		if (num & 0x80)
		{
			TDIN = 1;  
		}
		else 
		{
			TDIN = 0;   
		}
		num <<= 1;    
		TCLK = 0; 
		delay_us(1);
		TCLK = 1;    //上升沿有效	        
	}		 			    
} 		 


/*************************************************************
*Function Name  : TP_Read_AD
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-30
*Description    : 从触摸屏IC读取adc值
*Input          ：CMD  指令
*Output         ：
*Return         ：读到的数据
*************************************************************/
u16 TP_Read_AD(u8 CMD)	  
{ 	 
	u8 count = 0; 	  
	u16 Num = 0; 
	TCLK=0;	           //先拉低时钟 	 
	TDIN=0; 	       //拉低数据线
	TCS=0; 		       //选中触摸屏IC
	TP_Write_Byte(CMD);//发送命令字
	delay_us(6);       //ADS7846的转换时间最长为6us
	TCLK=0; 	     	    
	delay_us(1);    	   
	TCLK=1;		       //给1个时钟，清除BUSY
	delay_us(1);    
	TCLK=0; 
    /*****读出16位数据,只有高12位有效*****/	
	for (count=0; count<16; count++)
	{ 				  
		Num <<= 1; 	 
		TCLK = 0;	   //下降沿有效  	    	   
		delay_us(1);    
 		TCLK = 1;
 		if (DOUT)
		{
			Num++; 		 
		}
	}  	
	Num >>= 4;   	   //只有高12位有效.
	TCS = 1;	       //释放片选	 
	return (Num);   
}


/*************************************************************
*Function Name  : TP_Read_XOY
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-30
*Description    : 1.读取一个坐标值(x或者y)
                  2.连续读取READ_TIMES次数据,对这些数据升序排列,
                    然后去掉最低和最高LOST_VAL个数,取平均值 
*Input          ：xy  指令（CMD_RDX/CMD_RDY）
*Output         ：
*Return         ：读到的数据
*************************************************************/
#define  READ_TIMES 5 	 //读取次数
#define  LOST_VAL 1	     //丢弃值
u16 TP_Read_XOY(u8 xy)
{
	u16 i, j;
	u16 buf[READ_TIMES];
	u16 sum = 0;
	u16 temp;
	
	/*******取值*******/
	for (i=0; i<READ_TIMES; i++)
	{
		buf[i] = TP_Read_AD(xy);		 		    
	}
	
	/******升序排列******/
	for (i=0; i<READ_TIMES-1; i++)
	{
		for (j=i+1; j<READ_TIMES; j++)
		{
			if (buf[i] > buf[j])
			{
				temp = buf[i];
				buf[i] = buf[j];
				buf[j] = temp;
			}
		}
	}	  
	
	for (i=LOST_VAL; i<READ_TIMES-LOST_VAL; i++)
	{
		sum += buf[i];
	}
	temp = sum / (READ_TIMES - 2*LOST_VAL);
	return temp;   
} 


/*************************************************************
*Function Name  : TP_Read_XY
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-30
*Description    : 1.读取x,y坐标
                  2.连续读取READ_TIMES次数据,对这些数据升序排列,
                    然后去掉最低和最高LOST_VAL个数,取平均值 
*Input          ：1. *x  x轴坐标存储区
                  2. *y  y轴坐标存储区
*Output         ：
*Return         ：1,成功  0,失败
*************************************************************/
u8 TP_Read_XY(u16 *x,u16 *y)
{
	u16 xtemp,ytemp;			 	 		  
	xtemp = TP_Read_XOY(CMD_RDX);
	ytemp = TP_Read_XOY(CMD_RDY);	  												   
	*x = xtemp;
	*y = ytemp;
	return 1;//读数成功
}


/*************************************************************
*Function Name  : TP_Read_XY2
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-30
*Description    : 1.连续2次读取触摸屏IC,且这两次的偏差不能超过
                    ERR_RANGE,满足条件,则认为读数正确,否则读数错误.
                  2.该函数能大大提高准确度 
*Input          ：1. *x  x轴坐标存储区
                  2. *y  y轴坐标存储区
*Output         ：
*Return         ：1,成功  0,失败
*************************************************************/
#define  ERR_RANGE  50   //误差范围 
u8 TP_Read_XY2(u16 *x, u16 *y) 
{
	u16 x1, y1;
 	u16 x2, y2;
 	u8 flag;  
	
    flag = TP_Read_XY(&x1, &y1);   
    if(flag==0)
	{
		return 0;       //读取失败
	}
	
    flag = TP_Read_XY(&x2, &y2);	   
    if(flag==0)
	{
		return 0;       //读取失败
	}
	
	/*******前后两次采样在+-50内*******/
    if(( (x2<=x1 && x1<x2+ERR_RANGE) || (x1<=x2 && x2<x1+ERR_RANGE) )
    && ( (y2<=y1 && y1<y2+ERR_RANGE) || (y1<=y2 && y2<y1+ERR_RANGE) ))
    {
        *x = (x1 + x2) / 2;
        *y = (y1 + y2) / 2;
        return 1;
    }
	else 
	{
		return 0;	  
	}
}  		  

/*************************************************************
*Function Name  : TP_Drow_Touch_Point
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-30
*Description    : 画一个触摸点（用来校准用的）
*Input          ：1. x      x轴坐标
                  2. y      y轴坐标
                  3. color  点的颜色
*Output         ：
*Return         ：
*************************************************************/
void TP_Drow_Touch_Point(u16 x,u16 y,u16 color)
{
	POINT_COLOR=color;
	LCD_DrawLine(x-12,y,x+13,y);//横线
	LCD_DrawLine(x,y-12,x,y+13);//竖线
	LCD_DrawPoint(x+1,y+1);
	LCD_DrawPoint(x-1,y+1);
	LCD_DrawPoint(x+1,y-1);
	LCD_DrawPoint(x-1,y-1);
	LCD_Draw_Circle(x,y,6);//画中心圈
}	  

/*************************************************************
*Function Name  : TP_Draw_Big_Point
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-30
*Description    : 画一个大点(2*2的点)
*Input          ：1. x      x轴坐标
                  2. y      y轴坐标
                  3. color  点的颜色
*Output         ：
*Return         ：
*************************************************************/
void TP_Draw_Big_Point(u16 x,u16 y,u16 color)
{	    
	POINT_COLOR=color;
	LCD_DrawPoint(x,y);  //中心点 
	LCD_DrawPoint(x+1,y);
	LCD_DrawPoint(x,y+1);
	LCD_DrawPoint(x+1,y+1);	 	  	
}						  


/*************************************************************
*Function Name  : Load_Drow_Dialog
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-05-01
*Description    : 清空屏幕并在右上角显示"CLR"
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void Load_Drow_Dialog(void)
{
	LCD_Clear(WHITE);  //清屏   
 	POINT_COLOR=BLUE;  //设置字体为蓝色 
	LCD_ShowString(lcddev.width-24, 0, 24, 16, 16, (u8*)"CLR");//显示清屏区域
  	POINT_COLOR=RED;   //设置画笔蓝色 
}


/*************************************************************
*Function Name  : TP_Scan
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-30
*Description    : 触摸按键扫描
*Input          ：tp  
				  @arg 0  屏幕坐标
				  @arg 1  物理坐标(校准等特殊场合用)
*Output         ：
*Return         ：1,触屏有触摸 0,触屏无触摸
*************************************************************/
u8 TP_Scan(u8 tp)
{
	/****** 有按键按下 ******/
	if (PEN == 0)  
	{
		if (tp)
		{
			TP_Read_XY2(&tp_dev.x[0], &tp_dev.y[0]); //读取物理坐标
		}
		
		/****** 读取物理坐标，并将其装换成屏幕坐标 ******/
		else if (TP_Read_XY2(&tp_dev.x[0],&tp_dev.y[0]))   
		{
	 		tp_dev.x[0]=tp_dev.xfac*tp_dev.x[0]+tp_dev.xoff;
			tp_dev.y[0]=tp_dev.yfac*tp_dev.y[0]+tp_dev.yoff;  
	 	} 
		
		/****** 之前没有被按下 ******/
		if ( (tp_dev.sta & TP_PRES_DOWN) == 0 )      
		{		 
			tp_dev.sta = TP_PRES_DOWN | TP_CATH_PRES; //状态位8、7同时置1 
			
			/**记录第一次按下时的坐标**/
			tp_dev.x[4]=tp_dev.x[0];                 
			tp_dev.y[4]=tp_dev.y[0];  	   			 
		}			   
	}
	
	/****** 无按键按下 ******/
	else
    {
		if(tp_dev.sta & TP_PRES_DOWN) //之前是被按下的
		{
			tp_dev.sta &= ~(1<<7);    //标记触摸屏松开	
		}
		
		
		else                          //之前就没有被按下
		{
			tp_dev.x[4]=0;
			tp_dev.y[4]=0;
			tp_dev.x[0]=0xff;
			tp_dev.y[0]=0xff;
		}		
	}
	
	return (tp_dev.sta & TP_PRES_DOWN);//返回当前的触屏状态
}	  


/*************************************************************
*Function Name  : TP_Save_Adjdata
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-30
*Description    : 保存触摸屏校准参数
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
//保存FLASH里面的地址区间基址,占用18个字节(RANGE:SAVE_ADDR_BASE~SAVE_ADDR_BASE+17)
#define  SAVE_ADDR_BASE  1
void TP_Save_Adjdata(void)
{
	u8 result_data[18];  //存储缓冲区
	int temp = 0;

	/******************校正结果转换为存储数据******************/
	/*x校正因素*/
    temp = tp_dev.xfac*100000000;
	result_data[0] =  (u8)(temp >> 24);
	result_data[1] =  (u8)(temp >> 16);
	result_data[2] =  (u8)(temp >> 8);
	result_data[3] =  (u8)temp;
	/*y校正因素*/
	temp = tp_dev.yfac * 100000000;    
    result_data[4] =  (u8)(temp >> 24);
	result_data[5] =  (u8)(temp >> 16);
	result_data[6] =  (u8)(temp >> 8);
	result_data[7] =  (u8)temp;
	/*x偏移量*/
	temp = tp_dev.xoff * 100000;    
    result_data[8] =  (u8)(temp >> 24);
	result_data[9] =  (u8)(temp >> 16);
	result_data[10] = (u8)(temp >> 8);
	result_data[11] = (u8)temp;
	/*y偏移量*/
	temp = tp_dev.yoff * 100000;    
    result_data[12] = (u8)(temp >> 24);
	result_data[13] = (u8)(temp >> 16);
	result_data[14] = (u8)(temp >> 8);
	result_data[15] = (u8)temp;
	/*触屏类型*/
	temp = tp_dev.touchtype;
	result_data[16] = (u8)temp;
    /*标记校准过了*/
	temp=0x0A;
	result_data[17] = (u8)temp;
	
	W25QXX_Write(result_data, SAVE_ADDR_BASE, 18);  //校正结果写入FLASH
}


/*************************************************************
*Function Name  : TP_Get_Adjdata
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-30
*Description    : 得到保存在FLASH里面的校准值
*Input          ：
*Output         ：
*Return         ：1，成功获取数据    0，获取失败，要重新校准
*************************************************************/
u8 TP_Get_Adjdata(void)
{					
	u8 result_data[18];  //存储缓冲区
	int temp = 0;

	W25QXX_Read(result_data, SAVE_ADDR_BASE, 18);  //读取FLASH中的数据
	
	temp = result_data[17]; //读取标记字,看是否校准过！ 
	
	/***********触摸屏已经校准过了***********/
	if (temp == 0x0A)			   
	{
		/*得到x校准参数*/
		temp = result_data[0];
		temp = (temp << 8) + result_data[1];
		temp = (temp << 8) + result_data[2];
		temp = (temp << 8) + result_data[3];
		tp_dev.xfac = (float)temp / 100000000;
		/*得到y校准参数*/
		temp = result_data[4];
		temp = (temp << 8) + result_data[5];
		temp = (temp << 8) + result_data[6];
		temp = (temp << 8) + result_data[7];
		tp_dev.yfac = (float)temp / 100000000;
        /*得到x偏移量*/
        temp = result_data[8];
		temp = (temp << 8) + result_data[9];
		temp = (temp << 8) + result_data[10];
		temp = (temp << 8) + result_data[11];
		tp_dev.xoff = (float)temp / 100000;
		/*得到y偏移量*/
        temp = result_data[12];
		temp = (temp << 8) + result_data[13];
		temp = (temp << 8) + result_data[14];
		temp = (temp << 8) + result_data[15];
		tp_dev.yoff = (float)temp / 100000;
		/*得到触屏类型标记*/
 		temp = result_data[16];		
 		tp_dev.touchtype = temp;
		if(tp_dev.touchtype)   //X,Y方向与屏幕相反
		{
			CMD_RDX=0X90;
			CMD_RDY=0XD0;	 
		}else				   //X,Y方向与屏幕相同
		{
			CMD_RDX=0XD0;
			CMD_RDY=0X90;	 
		}		 
		return 1;	 
	}
	return 0;  //触摸屏未校准
}	 


/*************************************************************
*Function Name  : TP_Adj_Info_Show
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-05-01
*Description    : 提示校准结果(各个参数)
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
//提示字符串
u8* const TP_REMIND_MSG_TBL = (u8*)"Please use the stylus click the cross on the screen.The cross will always move until the screen adjustment is completed.";
void TP_Adj_Info_Show(u16 x0, u16 y0, u16 x1, u16 y1, u16 x2, u16 y2, u16 x3, u16 y3, u16 fac)
{	  
	POINT_COLOR = RED;
	LCD_ShowString(40, 160, lcddev.width, lcddev.height, 16, (u8*)"x1:");
 	LCD_ShowString(40+80, 160, lcddev.width, lcddev.height, 16, (u8*)"y1:");
 	LCD_ShowString(40, 180, lcddev.width, lcddev.height,16,(u8*)"x2:");
 	LCD_ShowString(40+80, 180, lcddev.width, lcddev.height, 16, (u8*)"y2:");
	LCD_ShowString(40,200, lcddev.width, lcddev.height, 16, (u8*)"x3:");
 	LCD_ShowString(40+80, 200, lcddev.width, lcddev.height, 16, (u8*)"y3:");
	LCD_ShowString(40,220, lcddev.width, lcddev.height, 16, (u8*)"x4:");
 	LCD_ShowString(40+80, 220, lcddev.width, lcddev.height, 16, (u8*)"y4:");  
 	LCD_ShowString(40,240, lcddev.width, lcddev.height, 16, (u8*)"fac is:");     
	LCD_ShowNum(40+24, 160, x0, 4, 16);		//显示数值
	LCD_ShowNum(40+24+80, 160, y0, 4, 16);	//显示数值
	LCD_ShowNum(40+24, 180, x1, 4, 16);		//显示数值
	LCD_ShowNum(40+24+80, 180, y1, 4, 16);	//显示数值
	LCD_ShowNum(40+24, 200, x2, 4, 16);		//显示数值
	LCD_ShowNum(40+24+80, 200, y2, 4, 16);	//显示数值
	LCD_ShowNum(40+24, 220, x3, 4, 16);     //显示数值
	LCD_ShowNum(40+24+80, 220, y3, 4, 16);	//显示数值
 	LCD_ShowNum(40+56, 240, fac, 3, 16);	//显示数值,该数值必须在95~105范围之内.

}


/*************************************************************
*Function Name  : TP_Adjust
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-30
*Description    : 触摸屏校准  得到四个校准参数
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void TP_Adjust(void)
{								 
	u16 pos_temp[4][2];  //坐标缓存值
	u8  cnt = 0;	
	u16 d1, d2;
	u32 tem1, tem2;
	double fac; 	
	u16 outtime = 0;
 				
	POINT_COLOR = BLUE;
	BACK_COLOR  = WHITE;
	
	LCD_Clear(WHITE);   //清屏   
	POINT_COLOR=RED;    //字体红色 
	LCD_Clear(WHITE);   //清屏 	   
	POINT_COLOR=BLACK;  //字体黑色
	LCD_ShowString(40,40,160,100,16,(u8*)TP_REMIND_MSG_TBL);//显示提示信息
	
	TP_Drow_Touch_Point(20, 20, RED);      //画点1 
	tp_dev.sta=0;      //消除触发信号 
	tp_dev.xfac=0;     //xfac用来标记是否校准过,所以校准之前必须清掉!以免错误	 
	while (1)          //如果连续10秒钟没有按下,则自动退出
	{
		tp_dev.scan(1);           //扫描物理坐标
		
		/*** 按键按下了一次后(此时按键松开了)的处理程序 ***/
		if ((tp_dev.sta&0xc0) == TP_CATH_PRES)
		{	
			outtime=0;		
			tp_dev.sta &= ~(1<<6);//标记按键已经被处理过了.
						   			   
			pos_temp[cnt][0]=tp_dev.x[0];
			pos_temp[cnt][1]=tp_dev.y[0];
			cnt++;
			
			switch(cnt)
			{			   
				case 1:						 
					TP_Drow_Touch_Point(20,20,WHITE);				          //清除点1 
					TP_Drow_Touch_Point(lcddev.width-20,20,RED);	          //画点2
					break;
				case 2:
 					TP_Drow_Touch_Point(lcddev.width-20,20,WHITE);	          //清除点2
					TP_Drow_Touch_Point(20,lcddev.height-20,RED);	          //画点3
					break;
				case 3:
 					TP_Drow_Touch_Point(20,lcddev.height-20,WHITE);           //清除点3
 					TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,RED);//画点4
					break;
				case 4:	 //全部四个点已经得到
					//对边相等
				    /*** 得到1,2的距离 ***/
					tem1 = abs(pos_temp[0][0] - pos_temp[1][0]);  //x1-x2
					tem2 = abs(pos_temp[0][1] - pos_temp[1][1]);  //y1-y2
					tem1 *= tem1;
					tem2 *= tem2;
					d1 = sqrt(tem1+tem2);
					
				    /*** 得到3,4的距离 ***/
					tem1 = abs(pos_temp[2][0]-pos_temp[3][0]);//x3-x4
					tem2 = abs(pos_temp[2][1]-pos_temp[3][1]);//y3-y4
					tem1 *= tem1;
					tem2 *= tem2;
					d2=sqrt(tem1+tem2);  
				
					fac = (float)d1/d2;
				
				    /*** 校准不合格 则继续校准 ***/
					if (fac<0.95 || fac>1.05 || d1==0 || d2==0)
					{
						cnt = 0;
 				    	TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);//清除点4
   	 					TP_Drow_Touch_Point(20,20,RED);								//画点1
 						TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//显示数据   
 						continue;
					}//水平线合格
					
					/*** 得到1,3的距离 ***/
					tem1 = abs(pos_temp[0][0]-pos_temp[2][0]);//x1-x3
					tem2 = abs(pos_temp[0][1]-pos_temp[2][1]);//y1-y3
					tem1 *= tem1;
					tem2 *= tem2;
					d1 = sqrt(tem1 + tem2);
					
					/*** 得到2,4的距离 ***/
					tem1 = abs(pos_temp[1][0]-pos_temp[3][0]);//x2-x4
					tem2 = abs(pos_temp[1][1]-pos_temp[3][1]);//y2-y4
					tem1 *= tem1;
					tem2 *= tem2;
					d2 = sqrt(tem1 + tem2);
					
					fac = (float)d1/d2;
					
					/*** 校准不合格 则继续校准 ***/
					if (fac<0.95 || fac>1.05)
					{
						cnt=0;
 				    	TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);//清除点4
   	 					TP_Drow_Touch_Point(20,20,RED);								//画点1
 						TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//显示数据   
						continue;
					}//对角线合格
					
					/*** 得到2,3的距离 ***/					
					tem1 = abs(pos_temp[1][0]-pos_temp[2][0]);//x2-x3
					tem2 = abs(pos_temp[1][1]-pos_temp[2][1]);//y2-y3
					tem1 *= tem1;
					tem2 *= tem2;
					d1 = sqrt(tem1 + tem2);
	                 
					/*** 得到1,4的距离 ***/		
					tem1 = abs(pos_temp[0][0]-pos_temp[3][0]);//x1-x4
					tem2 = abs(pos_temp[0][1]-pos_temp[3][1]);//y1-y4
					tem1 *= tem1;
					tem2 *= tem2;
					d2 = sqrt(tem1 + tem2);
					
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05)//不合格
					{
						cnt=0;
 				    	TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);//清除点4
   	 					TP_Drow_Touch_Point(20,20,RED);								//画点1
 						TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//显示数据   
						continue;
					}//垂直线合格
					
					/*** 开始计算结果 ***/
					tp_dev.xfac=(float)(lcddev.width-40)/(pos_temp[1][0]-pos_temp[0][0]);     //得到xfac		 
					tp_dev.xoff=(lcddev.width-tp_dev.xfac*(pos_temp[1][0]+pos_temp[0][0]))/2; //得到xoff
						  
					tp_dev.yfac=(float)(lcddev.height-40)/(pos_temp[2][1]-pos_temp[0][1]);    //得到yfac
					tp_dev.yoff=(lcddev.height-tp_dev.yfac*(pos_temp[2][1]+pos_temp[0][1]))/2;//得到yoff  
					
					/********* 触屏和预设的相反了 *********/
					if (abs(tp_dev.xfac)>2 || abs(tp_dev.yfac)>2)
					{
						cnt=0;
 				    	TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//清除点4
   	 					TP_Drow_Touch_Point(20,20,RED);								    //画点1
						LCD_ShowString(40, 26, lcddev.width, lcddev.height, 16, (u8*)"TP Need readjust!");
						tp_dev.touchtype = !tp_dev.touchtype;   //修改触屏类型.
						
						if(tp_dev.touchtype)  //X,Y方向与屏幕相反
						{
							CMD_RDX=0X90;
							CMD_RDY=0XD0;	 
						}
						else				  //X,Y方向与屏幕相同
						{
							CMD_RDX=0XD0;
							CMD_RDY=0X90;	 
						}			    
						continue;
					}		
					
					POINT_COLOR=BLUE;
					LCD_Clear(WHITE);  //清屏
					LCD_ShowString(35, 110, lcddev.width, lcddev.height, 16, (u8*)"Touch Screen Adjust OK!");//校正完成
					delay_ms(1000);
					TP_Save_Adjdata();  
 					LCD_Clear(WHITE);  //清屏   
					return;            //校正完成				 
			}
		}
		
		delay_ms(1);
		outtime++; 
		if(outtime>10000)
		{
			TP_Get_Adjdata();
			break;
	 	}
	}
}	


/*************************************************************
*Function Name  : TP_Init
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-30
*Description    : 触摸屏初始化  
                  包括W25Q16、SPI接口（软件模拟）、T_PEN等
*Input          ：
*Output         ：
*Return         ：1,进行过校准  0,没有进行校准
*************************************************************/
u8 TP_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;	
	EXTI_InitTypeDef EXTI_PT_InitStructure;
	NVIC_InitTypeDef NVIC_PT_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);  //使能SYCFG时钟

    /****** PCin(5)   T_PEN ******/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;      //输入模式
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;      //上拉
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);         

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource5);	
	
	EXTI_PT_InitStructure.EXTI_Line = EXTI_Line5;  
	EXTI_PT_InitStructure.EXTI_LineCmd = ENABLE;                  //使能
	EXTI_PT_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;        //中断事件
	EXTI_PT_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;    //下降沿触发
	EXTI_Init(&EXTI_PT_InitStructure);
	
	NVIC_PT_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_PT_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_PT_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//抢占优先级2
	NVIC_PT_InitStructure.NVIC_IRQChannelSubPriority = 0x00;       //字优先级0
	NVIC_Init(&NVIC_PT_InitStructure);

	/****** PBin(14)  T_MISO ******/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;      //输入模式
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;      //上拉
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);           

    /*** T_MOSI:PBout(15) T_SCK:PBout(13) T_CS:PBout(12) ***/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15 | GPIO_Pin_13 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;     //输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;    //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);           	

	W25QXX_Init();		                    //初始化W25Q16
	TP_Read_XY(&tp_dev.x[0], &tp_dev.y[0]); //第一次读取初始化
	
	if ( TP_Get_Adjdata() )
	{
		return 0;                           //成功获取校准数据
	}
	
	else			                        //未校准或获取校准数据失败
	{ 										    
		LCD_Clear(WHITE);                   //清屏
		TP_Adjust();  	                    //屏幕校准 
		TP_Save_Adjdata();	 
	}			
	TP_Get_Adjdata();	
	
	return 1; 									 
}


/*************************************************************
*Function Name  : EXTI9_5_IRQHandler
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-04-30
*Description    : 如果屏幕被按下 T_PEN引脚触发该中断  
                  传递事件标志 屏幕开始扫描触摸点坐标
*Input          ：
*Output         ：
*Return         ：
*************************************************************/
void EXTI9_5_IRQHandler(void)
{
	u8 errEXTI9_5_IRQHandler;
#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif
	
	if (EXTI_GetFlagStatus(EXTI_Line5) == SET)
	{
		EXTI_ClearFlag(EXTI_Line5);  //清除Lin5上的标志位
		OSFlagPost(flagTP, 0x01, OS_FLAG_SET, &errEXTI9_5_IRQHandler);
	}
	
#if SYSTEM_SUPPORT_OS 	           //如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif
}
