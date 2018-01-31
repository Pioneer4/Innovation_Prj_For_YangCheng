/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : CO2.h
*Auther         : 张沁
*Vertion        : v1.0
*Date           : 2017-10-10
*Description    : 该文件包含了CO2.c中所有的函数原型
                  该文件为移植文件，源于DFROBOT
*Function List  : 
*************************************************************/
#ifndef   __CO2_H_
#define   __CO2_H_

#include  "myinclude.h"

/************************Hardware Related Macros************************************/
//#define         DC_GAIN                      (8.5)   //define the DC gain of amplifier

/***********************Software Related Macros************************************/
#define         READ_SAMPLE_INTERVAL         (50)    //define how many samples you are going to take in normal operation
//#define         READ_SAMPLE_TIMES            (5)     //define the time interval(in milisecond) between each samples in

/**********************Application Related Macros**********************************/
//These two values differ from sensor to sensor. user should derermine this value.
//#define         ZERO_POINT_VOLTAGE           (0.324) //define the output of the sensor in volts when the concentration of CO2 is 400PPM
//#define         REACTION_VOLTGAE             (0.020) //define the voltage drop of the sensor when move the sensor from air into 1000ppm CO2

extern u16 CO2_value;
extern float   CO2Curve[3];
extern float MGRead(void);
extern int  MGGetPercentage(float volts);

#endif 
