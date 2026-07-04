#include "stdlib.h"
#include "motor.h"

float Velcity_Kp = 1.5f;
float Velcity_Ki = 0.6f;
float Velcity_Kd = 0.0f;

int MA_Target_Vel = 15;
int MB_Target_Vel = 15;

void Motor_Init(void){
	
	DL_Timer_startCounter(PWM_0_INST);
	DL_Timer_startCounter(PWM_1_INST);
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
�������ܣ������PID�ջ�����
��ڲ��������ҵ���ı�����ֵ
����ֵ  �������PWM
***************************************************************************/
int Velocity_A(int TargetVelocity, int CurrentVelocity)
{  
    int Bias;  //������ر���
		static int ControlVelocityA, Last_biasA; //��̬�������������ý�������ֵ��Ȼ����
		
		Bias=TargetVelocity-CurrentVelocity; //���ٶ�ƫ��
		
		ControlVelocityA+=Velcity_Ki*(Bias-Last_biasA)+Velcity_Kp*Bias;  //����ʽPI������
                                                                   //Velcity_Kp*(Bias-Last_bias) ����Ϊ���Ƽ��ٶ�
	                                                                 //Velcity_Ki*Bias             �ٶȿ���ֵ��Bias���ϻ��ֵõ� ƫ��Խ����ٶ�Խ��
		Last_biasA=Bias;	
	    if(ControlVelocityA>7000) ControlVelocityA=7000;
	    else if(ControlVelocityA<-7000) ControlVelocityA=-7000;
		return ControlVelocityA; //�����ٶȿ���ֵ
}

/***************************************************************************
�������ܣ������PID�ջ�����
��ڲ��������ҵ���ı�����ֵ
����ֵ  �������PWM
***************************************************************************/
int Velocity_B(int TargetVelocity, int CurrentVelocity)
{  
    int Bias;  //������ر���
		static int ControlVelocityB, Last_biasB; //��̬�������������ý�������ֵ��Ȼ����
		
		Bias=TargetVelocity-CurrentVelocity; //���ٶ�ƫ��
		
		ControlVelocityB+=Velcity_Ki*(Bias-Last_biasB)+Velcity_Kp*Bias;  //����ʽPI������
                                                                   //Velcity_Kp*(Bias-Last_bias) ����Ϊ���Ƽ��ٶ�
	                                                                 //Velcity_Ki*Bias             �ٶȿ���ֵ��Bias���ϻ��ֵõ� ƫ��Խ����ٶ�Խ��
		Last_biasB=Bias;	
	    if(ControlVelocityB>7000) ControlVelocityB=7000;
	    else if(ControlVelocityB<-7000) ControlVelocityB=-7000;
		return ControlVelocityB; //�����ٶȿ���ֵ
}

