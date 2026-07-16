#include "encoder.h"

// ============ 编码器参数 ============
#define ENCODER_PPR 500   // 编码器线数
#define GEAR_RATIO 28     // 减速比
#define ENCODER_MULT 2    // 双边沿计数，2倍频
#define WHEEL_DIA_MM 65   // 轮子直径(mm) 实测
#define WHEEL_CIRC_MM 204 // 实测轮子周长(mm)
#define COUNTS_PER_WHEEL_REV (ENCODER_PPR * ENCODER_MULT * GEAR_RATIO) // 500*2*28=28000
// 速度公式: speed(mm/s) = diff * WHEEL_CIRC_MM / 140

// ============ 全局变量 ============
volatile int16_t speedA = 0, speedB = 0;      //单位: mm/s (始终为正，PID用)
volatile uint32_t debug_cntA = 0, debug_cntB = 0; // 调试：原始硬件计数值

// ============ 累计距离 ============
static int32_t total_countA = 0, total_countB = 0; // 累计编码器脉冲数
volatile int32_t distA_mm = 0, distB_mm = 0;       // 累计距离(mm，有符号)

// ============ PWM方向（由Set_PWM设置）============
static int8_t pwm_dirA = 1, pwm_dirB = 1; // +1前进, -1后退, 0停止

// ============ 初始化函数 ============
void Encoder_Init(void)
{
  // 清零硬件计数器
  DL_TimerG_setTimerCount(TIMG8, 0);
  DL_TimerG_setTimerCount(TIMG12, 0);
}

// ============ 编码器采样（已禁用）============
void Encoder_Sample(void)
{
  // 不再使用
}


// ============ 设置PWM方向（在Set_PWM中调用）============
void Encoder_SetPwmDir(int pwmL, int pwmR)
{
    // PWM为0时保持上次方向（避免停车时方向丢失）
    if (pwmL > 0)      pwm_dirA = 1;
    else if (pwmL < 0)  pwm_dirA = -1;
    // pwmL == 0 时 pwm_dirA 保持不变

    if (pwmR > 0)      pwm_dirB = 1;
    else if (pwmR < 0)  pwm_dirB = -1;
}

// ============ 速度计算（主循环 10ms 周期调用）============
// 速度始终为正（PID用），距离带符号（方向由PWM命令判断）
void Encoder_CalcSpeed(void)
{
  uint32_t cntA = DL_TimerG_getTimerCount(TIMG8);
  DL_TimerG_setTimerCount(TIMG8, 0);
  debug_cntA = cntA;
  speedA = (int16_t)((int32_t)cntA * WHEEL_CIRC_MM / 140);
  total_countA += (int32_t)cntA * pwm_dirA;
  distA_mm = total_countA * WHEEL_CIRC_MM / COUNTS_PER_WHEEL_REV;

  uint32_t cntB = DL_TimerG_getTimerCount(TIMG12);
  DL_TimerG_setTimerCount(TIMG12, 0);
  debug_cntB = cntB;
  speedB = (int16_t)((int32_t)cntB * WHEEL_CIRC_MM / 140);
  total_countB += (int32_t)cntB * pwm_dirB;
  distB_mm = total_countB * WHEEL_CIRC_MM / COUNTS_PER_WHEEL_REV;
}

// ============ 清零累计距离 ============
void Encoder_ResetDist(void)
{
  total_countA = 0;
  total_countB = 0;
  distA_mm = 0;
  distB_mm = 0;
}

