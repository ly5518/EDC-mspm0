#include "encoder.h"

// ============ 编码器参数 ============
#define ENCODER_PPR 500   // 编码器线数
#define GEAR_RATIO 28     // 减速比
#define WHEEL_RADIUS_M 65 // 轮子半径(mm)
#define WHEEL_CIRCUMFERENCE (2.0f * 3.1415926f * WHEEL_RADIUS_M)
#define COUNTS_PER_WHEEL_REV (ENCODER_PPR * GEAR_RATIO)
#define SPEED_CALC_PERIOD 500  // 每500次采样算一次速度（100us x 500 = 50ms）

// ============ 全局变量（主循环直接读取）============
volatile float speedA = 0, speedB = 0;

static int32_t encoder_countA = 0, encoder_countB = 0;
static uint16_t sample_cnt = 0;

// ============ 初始化函数 ============
void Encoder_Init(void)
{
  DL_GPIO_initDigitalInputFeatures(IOMUX_PINCM55,
      DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
      DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
  DL_GPIO_initDigitalInputFeatures(IOMUX_PINCM52,
      DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
      DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
}

// ============ 编码器采样+速度计算（在TIMER_1 ISR中调用，每100us）============
// 每次调用：读硬件计数器累加
// 每100次（10ms）：计算速度并更新全局变量
void Encoder_Sample(void)
{
  // ---- 累加硬件计数 ----
  int16_t cntA = (int16_t)DL_TimerG_getTimerCount(TIMG8);
  DL_TimerG_setTimerCount(TIMG8, 0);
  encoder_countA += cntA;

  int16_t cntB = (int16_t)DL_TimerG_getTimerCount(TIMG12);
  DL_TimerG_setTimerCount(TIMG12, 0);
  encoder_countB += cntB;

  // ---- 每10ms计算一次速度 ----
  if (++sample_cnt >= SPEED_CALC_PERIOD)
  {
    sample_cnt = 0;

    static int32_t last_countA = 0, last_countB = 0;

    int32_t diffA = encoder_countA - last_countA;
    last_countA = encoder_countA;
    speedA = (float)diffA / COUNTS_PER_WHEEL_REV * WHEEL_CIRCUMFERENCE / 0.05f;  // 50ms

    int32_t diffB = encoder_countB - last_countB;
    last_countB = encoder_countB;
    speedB = (float)diffB / COUNTS_PER_WHEEL_REV * WHEEL_CIRCUMFERENCE / 0.05f;  // 50ms
  }
}
