#ifndef _MOTOR_H
#define _MOTOR_H
#include "ti_msp_dl_config.h"
#include <stdint.h>

void Motor_Init(void);
void Set_PWM(int pwmL, int pwmR);

#endif
