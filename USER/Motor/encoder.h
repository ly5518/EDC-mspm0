#ifndef _ENCODER_H
#define _ENCODER_H
#include "ti_msp_dl_config.h"

void Encoder_Init(void);
void Encoder_Sample(void);
void Encoder_CalcSpeed(void);
void Encoder_SetPwmDir(int pwmL, int pwmR);
void Encoder_ResetDist(void);

extern volatile int16_t speedA, speedB;
extern volatile uint32_t debug_cntA, debug_cntB;
extern volatile int32_t distA_mm, distB_mm;

#endif
