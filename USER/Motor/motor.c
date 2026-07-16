/**
 * @file    motor.c
 * @brief   底层电机驱动：PWM输出
 */

#include "stdlib.h"
#include "motor.h"
#include "encoder.h"

void Motor_Init(void)
{
	DL_Timer_startCounter(PWM_0_INST);
	DL_Timer_startCounter(PWM_1_INST);
}

void Set_PWM(int pwmL, int pwmR)
{
	Encoder_SetPwmDir(-pwmL, -pwmR);

	pwmR = -pwmR;

	if(pwmL==0)
    {
		DL_Timer_setCaptureCompareValue(PWM_0_INST,0,GPIO_PWM_0_C0_IDX);
		DL_Timer_setCaptureCompareValue(PWM_0_INST,0,GPIO_PWM_0_C1_IDX);
    }
    else if(pwmL>0)
    {
		DL_Timer_setCaptureCompareValue(PWM_0_INST,abs(8000),GPIO_PWM_0_C0_IDX);
		DL_Timer_setCaptureCompareValue(PWM_0_INST,abs(8000-pwmL),GPIO_PWM_0_C1_IDX);
    }
    else
    {
		DL_Timer_setCaptureCompareValue(PWM_0_INST,abs(8000+pwmL),GPIO_PWM_0_C0_IDX);
		DL_Timer_setCaptureCompareValue(PWM_0_INST,abs(8000),GPIO_PWM_0_C1_IDX);
    }

	if(pwmR==0)
    {
		DL_Timer_setCaptureCompareValue(PWM_1_INST,0,GPIO_PWM_1_C0_IDX);
		DL_Timer_setCaptureCompareValue(PWM_1_INST,0,GPIO_PWM_1_C1_IDX);
    }
    else if(pwmR>0)
    {
		DL_Timer_setCaptureCompareValue(PWM_1_INST,abs(8000),GPIO_PWM_1_C0_IDX);
		DL_Timer_setCaptureCompareValue(PWM_1_INST,abs(8000-pwmR),GPIO_PWM_1_C1_IDX);
    }
    else
    {
		DL_Timer_setCaptureCompareValue(PWM_1_INST,abs(8000+pwmR),GPIO_PWM_1_C0_IDX);
		DL_Timer_setCaptureCompareValue(PWM_1_INST,abs(8000),GPIO_PWM_1_C1_IDX);
    }
}
