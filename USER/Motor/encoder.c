#include "encoder.h"

// ============ 编码器参数 ============
#define ENCODER_PPR 500   // 编码器线数
#define GEAR_RATIO 28     // 减速比
#define WHEEL_RADIUS_M 65 // 轮子半径(mm)
#define WHEEL_CIRCUMFERENCE (2.0f * 3.1415926f * WHEEL_RADIUS_M)
#define COUNTS_PER_WHEEL_REV (ENCODER_PPR * GEAR_RATIO)

// ============ 全局变量 ============
volatile int16_t dirA = 1, dirB = 1;
int32_t encoder_countA = 0, encoder_countB = 0;

// ============ 初始化函数 ============
void Encoder_Init(void)
{
  // ---- Encoder A: PA26 (E1B) TIMG8硬件计数, PA25 (E1A) GPIO方向 ----
  // SysConfig已配置TIMG8和PA26，这里只需配置PA25方向中断

  // PA25 GPIO输入
  DL_GPIO_initDigitalInputFeatures(IOMUX_PINCM55,
      DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
      DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

  // PA25 双边沿中断（方向检测）
  DL_GPIO_setUpperPinsPolarity(GPIOA, DL_GPIO_PIN_25_EDGE_RISE_FALL);
  DL_GPIO_clearInterruptStatus(GPIOA, DL_GPIO_PIN_25);
  DL_GPIO_enableInterrupt(GPIOA, DL_GPIO_PIN_25);

  // ---- Encoder B: PB20 (E2A) TIMG12硬件计数, PB24 (E2B) GPIO方向 ----
  // SysConfig已配置TIMG12和PB20，这里只需配置PB24方向中断

  // PB24 GPIO输入
  DL_GPIO_initDigitalInputFeatures(IOMUX_PINCM52,
      DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
      DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

  // PB24 双边沿中断（方向检测）
  DL_GPIO_setUpperPinsPolarity(GPIOB, DL_GPIO_PIN_24_EDGE_RISE_FALL);
  DL_GPIO_clearInterruptStatus(GPIOB, DL_GPIO_PIN_24 | KEY_up_PIN);
  DL_GPIO_enableInterrupt(GPIOB, DL_GPIO_PIN_24);

  // 使能NVIC中断
  NVIC_ClearPendingIRQ(GPIO_MULTIPLE_GPIOA_INT_IRQN);
  NVIC_ClearPendingIRQ(GPIO_MULTIPLE_GPIOB_INT_IRQN);
  NVIC_EnableIRQ(GPIO_MULTIPLE_GPIOA_INT_IRQN);
  NVIC_EnableIRQ(GPIO_MULTIPLE_GPIOB_INT_IRQN);
}

// ============ 中断处理函数 ============
// 只处理方向中断，计数由硬件定时器完成
void GROUP1_IRQHandler(void)
{
  // ---- GPIOA: Encoder A方向 (PA25) ----
  uint32_t statusA = DL_GPIO_getEnabledInterruptStatus(GPIOA, DL_GPIO_PIN_25);
  if (statusA & DL_GPIO_PIN_25)
  {
    dirA = DL_GPIO_readPins(GPIOA, DL_GPIO_PIN_25) ? 1 : -1;
    DL_GPIO_clearInterruptStatus(GPIOA, DL_GPIO_PIN_25);
  }

  // ---- GPIOB: Encoder B方向 (PB24) + 按键 ----
  uint32_t statusB = DL_GPIO_getEnabledInterruptStatus(GPIOB, DL_GPIO_PIN_24 | KEY_up_PIN);

  if (statusB & DL_GPIO_PIN_24)
  {
    dirB = DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_24) ? 1 : -1;
    DL_GPIO_clearInterruptStatus(GPIOB, DL_GPIO_PIN_24);
  }

  if (statusB & KEY_up_PIN)
  {
    DL_GPIO_clearInterruptStatus(GPIOB, KEY_up_PIN);
  }
}

// ============ 计算速度 (m/s) ============
float Encoder_GetSpeedA(float dt_ms)
{
  static int32_t last_countA = 0;

  // 读取TIMG8硬件计数器（PA26 E1B）
  int16_t temp = (int16_t)DL_TimerG_getTimerCount(TIMG8);
  DL_TimerG_setTimerCount(TIMG8, 0);
  encoder_countA += dirA * temp;

  int32_t diff = encoder_countA - last_countA;
  last_countA = encoder_countA;

  return (float)diff / COUNTS_PER_WHEEL_REV * WHEEL_CIRCUMFERENCE / (dt_ms / 1000.0f);
}

float Encoder_GetSpeedB(float dt_ms)
{
  static int32_t last_countB = 0;

  // 读取TIMG12硬件计数器（PB20 E2A）
  int16_t temp = (int16_t)DL_TimerG_getTimerCount(TIMG12);
  DL_TimerG_setTimerCount(TIMG12, 0);
  encoder_countB += dirB * temp;

  int32_t diff = encoder_countB - last_countB;
  last_countB = encoder_countB;

  return (float)diff / COUNTS_PER_WHEEL_REV * WHEEL_CIRCUMFERENCE / (dt_ms / 1000.0f);
}
