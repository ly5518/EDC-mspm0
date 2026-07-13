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
volatile int16_t speedA = 0, speedB = 0;      //单位: mm/s
volatile int8_t encoder_dirA = 1, encoder_dirB = -1; // +1正转, -1反转
volatile uint32_t debug_cntA = 0, debug_cntB = 0; // 调试：原始硬件计数值

// ============ 累计距离 ============
static int32_t total_countA = 0, total_countB = 0; // 累计编码器脉冲数
volatile int32_t distA_mm = 0, distB_mm = 0;       // 累计距离(mm)

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
// 读取硬件计数器并清零，累计距离
void Encoder_CalcSpeed(void)
{
  uint32_t cntA = DL_TimerG_getTimerCount(TIMG8);
  DL_TimerG_setTimerCount(TIMG8, 0);
  debug_cntA = cntA;
  int32_t rawA = (int32_t)cntA * encoder_dirA;
  speedA = (int16_t)(rawA * WHEEL_CIRC_MM / 140);
  total_countA += rawA;
  distA_mm = total_countA * WHEEL_CIRC_MM / COUNTS_PER_WHEEL_REV;

  uint32_t cntB = DL_TimerG_getTimerCount(TIMG12);
  DL_TimerG_setTimerCount(TIMG12, 0);
  debug_cntB = cntB;
  int32_t rawB = (int32_t)cntB * encoder_dirB;
  speedB = (int16_t)(rawB * WHEEL_CIRC_MM / 140);
  total_countB += rawB;
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

// ============ 方向更新（在方向引脚GPIO中断中调用）============
void Encoder_UpdateDir(void)
{
  // 高电平=正转(+1)，低电平=反转(-1)
  encoder_dirA = DL_GPIO_readPins(ENCODER1_DIR_PORT, ENCODER1_DIR_PIN_ENCODER1_PIN) ? 1 : -1;
  // 右编码器方向引脚接线相反，取反
  encoder_dirB = DL_GPIO_readPins(ENCODER2_DIR_PORT, ENCODER2_DIR_PIN_ENCODER2_PIN) ? -1 : 1;
}
