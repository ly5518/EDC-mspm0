#include "stdlib.h"
#include "motor.h"
#include "pid.h"
#include "encoder.h"

extern float yaw_angle;

/* ============ 三个 PID 实例 ============ */
PID_TypeDef pid_speedA;   // 左轮速度环
PID_TypeDef pid_speedB;   // 右轮速度环
PID_TypeDef pid_steer;    // 转向环（角度）

/* ============ 目标值（菜单可调）============ */
int16_t target_speed = 0;   // 目标速度 mm/s
float   target_yaw   = 0;  // 目标角度 deg

/* ============ 初始化 ============ */
void Motor_Init(void)
{
	DL_Timer_startCounter(PWM_0_INST);
	DL_Timer_startCounter(PWM_1_INST);
}

void Motor_PID_Init(void)
{
	// 速度环：增量式PID，参数待整定
	PID_Init(&pid_speedA, 1.5f, 0.6f, 0.0f,
	         /*deadzone*/ 5, /*max_out*/ 7000, /*integral*/ 5000);
	PID_Init(&pid_speedB, 1.5f, 0.6f, 0.0f,
	         5, 7000, 5000);

	// 转向环：位置式PID，参数待整定
	PID_Init(&pid_steer, 2.0f, 0.0f, 0.5f,
	         /*deadzone*/ 1, /*max_out*/ 2000, /*integral*/ 3000);
}

/* ============ PID 计算（10ms 周期调用）============ */
void Motor_PID_Calc(void)
{
	// 1. 转向环：yaw → steer_out
	pid_steer.measured = (int16_t)yaw_angle;
	int16_t steer_out = PID_Positional_Calc(&pid_steer);

	// 2. 速度环：编码器速度 → PWM，叠加转向补偿
	pid_speedA.measured = speedA;
	pid_speedA.target   = target_speed + steer_out;
	int16_t pwmA = PID_Incremental_Calc(&pid_speedA);

	pid_speedB.measured = speedB;
	pid_speedB.target   = target_speed - steer_out;
	int16_t pwmB = PID_Incremental_Calc(&pid_speedB);

	// 3. 输出到电机
	Set_PWM(pwmA, pwmB);
}

void Set_PWM(int pwmL,int pwmR)
{
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
