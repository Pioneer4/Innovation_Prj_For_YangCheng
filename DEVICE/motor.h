/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : motor.h
*Auther         : 张沁
*Version        : v1.0
*Date           : 2017-05-03
*Description    : 该文件包含了motor.c中所有的函数原型
*Function List  : 
*************************************************************/
#ifndef __MOTOR_H
#define __MOTOR_H

#include "myinclude.h"

#define MOTOR1_PIN1 PAout(4)
#define MOTOR1_PIN2 PAout(5)

#define MOTOR_FORWARD  2
#define MOTOR_BACKWARD 1
#define MOTOR_STOP     0

void TravelMotorInit(void);  //初始化行程电机
void TravelMotorCtrl(u8 runType);
void DcMotorInit(void);      //初始化直流电机
void Motor1SpeedOut(u16 motor1SpeedVal);
void Motor2SpeedOut(u16 motor2SpeedVal);
void Motor3SpeedOut(u16 motor3SpeedVal);
void Motor4SpeedOut(u16 motor4SpeedVal);


#endif
