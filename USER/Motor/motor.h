#ifndef _MOTOR_H
#define _MOTOR_H
#include "ti_msp_dl_config.h"

// PID参数（菜单可调）
extern float Velcity_Kp;
extern float Velcity_Ki;
extern float Velcity_Kd;

// 目标速度（菜单可调）
extern int MA_Target_Vel;
extern int MB_Target_Vel;

void Motor_Init(void);
int Velocity_A(int TargetVelocity, int CurrentVelocity);
int Velocity_B(int TargetVelocity, int CurrentVelocity);
void Set_PWM(int pwmL, int pwmR);
int limit_PWM(int value, int low, int high);

#endif