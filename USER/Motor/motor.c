#include "stdlib.h"
#include "motor.h"

float Velcity_Kp = 0.7f;
float Velcity_Ki = 0.05f;
float Velcity_Kd = 0.0f;

int MA_Target_Vel = 100;
int MB_Target_Vel = 100;

void Motor_Init(void){

	DL_Timer_startCounter(PWM_0_INST);
	DL_Timer_startCounter(PWM_1_INST);
	DL_TimerG_startCounter(PWM_SG90_INST);
}

int limit_PWM(int value,int low,int high)
{
	if(value>high) return high;
	else if(value<low) return low;
	else return value;
}



void Set_PWM(int pwmL,int pwmR)
{
	 if(pwmL>=0)
    {
		DL_Timer_setCaptureCompareValue(PWM_0_INST,abs(8000),GPIO_PWM_0_C0_IDX);
		DL_Timer_setCaptureCompareValue(PWM_0_INST,abs(8000-pwmL),GPIO_PWM_0_C1_IDX);

    }
    else
    {
		DL_Timer_setCaptureCompareValue(PWM_0_INST,abs(8000+pwmL),GPIO_PWM_0_C0_IDX);
		DL_Timer_setCaptureCompareValue(PWM_0_INST,abs(8000),GPIO_PWM_0_C1_IDX);

    }

	 if(pwmR>=0)
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

/***************************************************************************
函数功能：增量式PID闭环控制
入口参数：目标速度、左右电机的编码器值
返回值  ：电机控制PWM
***************************************************************************/
// PID静态变量（移到函数外部，方便重置）
static int ControlVelocityA = 0, Last_biasA = 0;
static int ControlVelocityB = 0, Last_biasB = 0;

void Velocity_Reset(void)
{
    ControlVelocityA = 0; Last_biasA = 0;
    ControlVelocityB = 0; Last_biasB = 0;
}

int Velocity_A(int TargetVelocity, int CurrentVelocity)
{
    int Bias;  //定义偏差变量

		Bias=TargetVelocity-CurrentVelocity; //计算速度偏差

		ControlVelocityA+=Velcity_Ki*(Bias-Last_biasA)+Velcity_Kp*Bias;  //增量式PI控制器
                                                                   //Velcity_Kp*(Bias-Last_bias) 作用为加速度控制
                                                                 //Velcity_Ki*Bias             速度控制值，Bias越大加速度越大 偏差越大速度越快
		Last_biasA=Bias;
	    if(ControlVelocityA>7000) ControlVelocityA=7000;
	    else if(ControlVelocityA<-7000) ControlVelocityA=-7000;
		return ControlVelocityA; //返回速度控制值
}

/***************************************************************************
函数功能：增量式PID闭环控制
入口参数：目标速度、左右电机的编码器值
返回值  ：电机控制PWM
***************************************************************************/
int Velocity_B(int TargetVelocity, int CurrentVelocity)
{
    int Bias;  //定义偏差变量

		Bias=TargetVelocity-CurrentVelocity; //计算速度偏差

		ControlVelocityB+=Velcity_Ki*(Bias-Last_biasB)+Velcity_Kp*Bias;  //增量式PI控制器
                                                                   //Velcity_Kp*(Bias-Last_bias) 作用为加速度控制
                                                                 //Velcity_Ki*Bias             速度控制值，Bias越大加速度越大 偏差越大速度越快
		Last_biasB=Bias;
	    if(ControlVelocityB>7000) ControlVelocityB=7000;
	    else if(ControlVelocityB<-7000) ControlVelocityB=-7000;
		return ControlVelocityB; //返回速度控制值
}

