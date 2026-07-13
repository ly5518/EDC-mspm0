/*
 * 立创开发板硬件例程
 * 立创开发板：www.lckfb.com
 * 技术支持：wiki.lckfb.com
 * 技术支持驻守论坛，任何技术问题欢迎随时学习交流
 * 技术支持社区：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 我们不生产代码，我们只是代码的搬运工
 */
#include "bsp_sg90.h"

unsigned int Servo_Angle = 0; // 当前角度

/******************************************************************
   占空比 范围 0 ~ (per-1)
   t = 0.5ms-------------------舵机转动到 0 度
   t = 1.0ms-------------------舵机转动到 45度
   t = 1.5ms-------------------舵机转动到 90度
   t = 2.0ms-------------------舵机转动到 135度
   t = 2.5ms-------------------舵机转动到180度
******************************************************************/

/******************************************************************
 * 函 数 名 称：Set_Servo_Angle
 * 函 数 说 明：设置舵机角度
 * 函 数 形 参：angle=要设置的角度，范围0-180
 * 函 数 返 回：无
 * 作       者：LCKFB
 * 备       注：无
******************************************************************/
//void Set_Servo_Angle(unsigned int angle)
//{
//    uint32_t period = 400;

//    if(angle > 180)
//    {
//        angle = 180; // 限制角度在0到180度之间
//    }

//    Servo_Angle = angle;

//    // 计算PWM占空比
//    // 0.5ms对应的计数 = 10
//    // 2.5ms对应的计数 = 50
//    float min_count = 10.0f;
//    float max_count = 50.0f;
//    float range = max_count - min_count;
//    float ServoAngle = min_count + (((float)angle / 180.0f) * range);

//    DL_TimerG_setCaptureCompareValue(PWM_SG90_INST, (unsigned int)(ServoAngle + 0.5f), GPIO_PWM_SG90_C0_IDX);
//}
void Set_Servo_Angle(unsigned int angle)
{
    uint32_t period = 2000;

    if(angle > 180)
    {
        angle = 180; // 限制角度在0到180度之间
    }

    Servo_Angle = angle;

    // 计算PWM占空比
    // 0.5ms对应的计数 = 50
    // 2.5ms对应的计数 = 250
    float min_count = 50.0f;
    float max_count = 250.0f;
    float range = max_count - min_count;
    float ServoAngle = min_count + (((float)angle / 180.0f) * range);

    DL_TimerG_setCaptureCompareValue(PWM_SG90_INST, (unsigned int)(ServoAngle + 0.5f), GPIO_PWM_SG90_C0_IDX);
}
/******************************************************************
 * 函 数 名 称：获取当前角度
 * 函 数 说 明：Get_Servo_Angle
 * 函 数 形 参：无
 * 函 数 返 回：当前角度
 * 作       者：LCKFB
 * 备       注：使用前请确保之前使用过
                void Set_Servo_Angle(unsigned int angle)
                函数设置过角度
******************************************************************/
unsigned int Get_Servo_Angle(void)
{
    return Servo_Angle;
}
