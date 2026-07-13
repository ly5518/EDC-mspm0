#ifndef _MOTOR_H
#define _MOTOR_H
#include "ti_msp_dl_config.h"
#include <stdint.h>

/* 目标值（菜单可调） */
extern int16_t target_speed;  // mm/s
extern float   target_yaw;    // deg

void Motor_Init(void);
void Velocity_Reset(void);
void Motor_PID_Init(void);
void Motor_PID_Calc(void);
void Set_PWM(int pwmL, int pwmR);

#endif
