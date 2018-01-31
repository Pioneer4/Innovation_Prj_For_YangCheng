/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : CO2.c
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-10-10
*Description    : 用于对CO2模块数据的采集和处理
                  该文件为移植文件，数据转换算法部分源于DFROBOT
*Function List  : 
*************************************************************/
#include "myinclude.h"

const float  DC_GAIN = 8.5;             //define the DC gain of amplifier
const float  ZERO_POINT_VOLTAGE = 0.324;//define the output of the sensor in volts when the concentration of CO2 is 400PPM
const float  REACTION_VOLTGAE = 0.020;  //define the voltage drop of the sensor when move the sensor from air into 1000ppm CO2

const float  READ_SAMPLE_TIMES = 5.0;   //define the time interval(in milisecond) between each samples in


float   CO2Curve[3]  =  {2.602,ZERO_POINT_VOLTAGE,(REACTION_VOLTGAE/(2.602-3))}; 
u16 CO2_value;

/*****************************  MGRead *********************************************
Input:   mg_pin - analog channel
Output:  output of SEN0159
Remarks: This function reads the output of SEN0159
************************************************************************************/ 
float MGRead(void)
{
    int i;
    float v = 0;
    for (i=0; i<READ_SAMPLE_TIMES; i++) 
	{
        v += GetAdcAverage(5,1);
        delay_ms(READ_SAMPLE_INTERVAL);
    }

	v = (v / READ_SAMPLE_TIMES) * (float)3.3 / 4096;
    return v;  
}

/*****************************  MQGetPercentage **********************************
Input:   volts   - SEN-000007 output measured in volts
         pcurve  - pointer to the curve of the target gas
Output:  ppm of the target gas
Remarks: By using the slope and a point of the line. The x(logarithmic value of ppm) 
         of the line could be derived if y(MG-811 output) is provided. As it is a 
         logarithmic coordinate, power of 10 is used to convert the result to non-logarithmic 
         value.
************************************************************************************/ 
int  MGGetPercentage(float volts)
{
	if ( (volts/DC_GAIN ) >= ZERO_POINT_VOLTAGE )
	{
		return (-1);
	} 
	else
	{ 
		return pow(10, ((volts/DC_GAIN)-CO2Curve[1])/CO2Curve[2]+CO2Curve[0]);
	}
}
