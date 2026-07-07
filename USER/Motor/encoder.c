#include "encoder.h"
#include <math.h>

#define ENCODERB_INT_IRQN       (GPIO_MULTIPLE_GPIOB_INT_IRQN)

// 编码器参数
#define ENCODER_PPR             500     // 编码器线数
#define GEAR_RATIO              28      // 减速比
#define WHEEL_RADIUS_M          0.065f  // 轮子半径(m)
#define COUNTS_PER_MOTOR_REV    (ENCODER_PPR *4) // 4倍频，每电机转计数
#define COUNTS_PER_WHEEL_REV    (COUNTS_PER_MOTOR_REV * GEAR_RATIO) // 每轮转计数
#define WHEEL_CIRCUMFERENCE     (2.0f * 3.1415926f * WHEEL_RADIUS_M) // 轮子周长(m)

uint32_t gpio_interrup1, gpio_interrup2;
volatile int Get_Encoder_countA, Get_Encoder_countB;

void Encoder_Init(void)
{
	NVIC_ClearPendingIRQ(GPIO_MULTIPLE_GPIOA_INT_IRQN);
    NVIC_ClearPendingIRQ(ENCODERB_INT_IRQN);
	NVIC_EnableIRQ(GPIO_MULTIPLE_GPIOA_INT_IRQN);
    NVIC_EnableIRQ(ENCODERB_INT_IRQN);
	// NVIC_ClearPendingIRQ(TIMER_0_INST_INT_IRQN);
	// NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);
}
/*******************************************************
�������ܣ��ⲿ�ж�ģ��������ź�
��ں�������
����  ֵ����
***********************************************************/
void GROUP1_IRQHandler(void)
{
	//��ȡ�ж��ź�
    gpio_interrup1 = DL_GPIO_getEnabledInterruptStatus(ENCODERA_PORT,ENCODERA_E1A_PIN|ENCODERA_E1B_PIN);
    gpio_interrup2 = DL_GPIO_getEnabledInterruptStatus(ENCODERB_PORT,ENCODERB_E2A_PIN|ENCODERB_E2B_PIN);
    
    
	//encoderB
	if((gpio_interrup1 & ENCODERA_E1A_PIN)==ENCODERA_E1A_PIN)
	{
		if(!DL_GPIO_readPins(ENCODERA_PORT,ENCODERA_E1B_PIN))
		{
			Get_Encoder_countA--;
		}
		else
		{
			Get_Encoder_countA++;
		}
	}
	else if((gpio_interrup1 & ENCODERA_E1B_PIN)==ENCODERA_E1B_PIN)
	{
		if(!DL_GPIO_readPins(ENCODERA_PORT,ENCODERA_E1A_PIN))
		{
			Get_Encoder_countA++;
		}
		else
		{
			Get_Encoder_countA--;
		}
	}
	
	//encoderA
	if((gpio_interrup2 & ENCODERB_E2A_PIN)==ENCODERB_E2A_PIN)
	{
		if(!DL_GPIO_readPins(ENCODERB_PORT,ENCODERB_E2B_PIN))
		{
			Get_Encoder_countB--;
		}
		else
		{
			Get_Encoder_countB++;
		}
	}
	else if((gpio_interrup2 & ENCODERB_E2B_PIN)==ENCODERB_E2B_PIN)
	{
		if(!DL_GPIO_readPins(ENCODERB_PORT,ENCODERB_E2A_PIN))
		{
			Get_Encoder_countB++;
		}                 
		else              
		{                 
			Get_Encoder_countB--;
		}
	}
	DL_GPIO_clearInterruptStatus(ENCODERA_PORT,ENCODERA_E1A_PIN|ENCODERA_E1B_PIN);
	DL_GPIO_clearInterruptStatus(ENCODERB_PORT,ENCODERB_E2A_PIN|ENCODERB_E2B_PIN);

	// 清除按键引脚(PB21)的中断标志，防止按键中断未处理导致卡死
	DL_GPIO_clearInterruptStatus(GPIOB, KEY_up_PIN);
}

/*******************************************************
函数功能：获取电机A的速度(m/s)
入口参数：dt_ms - 采样时间间隔(ms)
返回值：速度值(m/s)，正负表示方向
***********************************************************/
float Encoder_GetSpeedA(float dt_ms)
{
    static int last_countA = 0;
    int current_count = Get_Encoder_countA;
    int diff = current_count - last_countA;
    last_countA = current_count;

    // 速度 = (计数差 / 每轮计数) * 周长 / 时间(s)
    float speed = (float)diff / COUNTS_PER_WHEEL_REV * WHEEL_CIRCUMFERENCE / (dt_ms / 1000.0f);
    return speed;
}

/*******************************************************
函数功能：获取电机B的速度(m/s)
入口参数：dt_ms - 采样时间间隔(ms)
返回值：速度值(m/s)，正负表示方向
***********************************************************/
float Encoder_GetSpeedB(float dt_ms)
{
    static int last_countB = 0;
    int current_count = Get_Encoder_countB;
    int diff = current_count - last_countB;
    last_countB = current_count;

    // 速度 = (计数差 / 每轮计数) * 周长 / 时间(s)
    float speed = (float)diff / COUNTS_PER_WHEEL_REV * WHEEL_CIRCUMFERENCE / (dt_ms / 1000.0f);
    return speed;
}
