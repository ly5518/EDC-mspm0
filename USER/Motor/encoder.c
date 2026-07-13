#include "encoder.h"

// ============ 编码器参数 ============
#define ENCODER_PPR 500   // 编码器线数
#define GEAR_RATIO 28     // 减速比
#define WHEEL_DIA_MM 65   // 轮子直径(mm)
#define WHEEL_CIRC_MM 204 //(3.14 * WHEEL_DIA_MM)=204
#define COUNTS_PER_WHEEL_REV (ENCODER_PPR * GEAR_RATIO) // 14000
// 速度公式: speed(mm/s) = diff * WHEEL_CIRC_MM / 140

// ============ 全局变量 ============
volatile int16_t speedA = 0, speedB = 0;      //单位: mm/s
volatile int8_t encoder_dirA = 1, encoder_dirB = 1; // +1正转, -1反转

// ============ 初始化函数 ============
void Encoder_Init(void)
{
  DL_GPIO_initDigitalInputFeatures(IOMUX_PINCM55,
      DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
      DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
  DL_GPIO_initDigitalInputFeatures(IOMUX_PINCM52,
      DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
      DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

  // 读取初始方向状态
  Encoder_UpdateDir();

  // 清零硬件计数器
  DL_TimerG_setTimerCount(TIMG8, 0);
  DL_TimerG_setTimerCount(TIMG12, 0);
}

// ============ 编码器采样（已禁用）============
void Encoder_Sample(void)
{
  // 不再使用
}

// ============ 速度计算（主循环 10ms 周期调用）============
// 读取硬件计数器并清零，低通滤波消除毛刺
void Encoder_CalcSpeed(void)
{
  uint32_t cntA = DL_TimerG_getTimerCount(TIMG8);
  DL_TimerG_setTimerCount(TIMG8, 0);
  int16_t rawA = (int16_t)(cntA * encoder_dirA * WHEEL_CIRC_MM / 140);

  uint32_t cntB = DL_TimerG_getTimerCount(TIMG12);
  DL_TimerG_setTimerCount(TIMG12, 0);
  int16_t rawB = (int16_t)(cntB * encoder_dirB * WHEEL_CIRC_MM / 140);

  // 一阶低通滤波：speed = speed * 7/8 + raw * 1/8
  // 消除周期性毛刺，响应时间约80ms
  speedA = (int16_t)((speedA * 7 + rawA) / 8);
  speedB = (int16_t)((speedB * 7 + rawB) / 8);
}

// ============ 方向更新（在方向引脚GPIO中断中调用）============
void Encoder_UpdateDir(void)
{
  // 高电平=正转(+1)，低电平=反转(-1)（与例程一致）
  encoder_dirA = DL_GPIO_readPins(ENCODER1_DIR_PORT, ENCODER1_DIR_PIN_ENCODER1_PIN) ? 1 : -1;
  encoder_dirB = DL_GPIO_readPins(ENCODER2_DIR_PORT, ENCODER2_DIR_PIN_ENCODER2_PIN) ? 1 : -1;
}
