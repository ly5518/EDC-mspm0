#ifndef _ENCODER_H
#define _ENCODER_H
#include "ti_msp_dl_config.h"

void Encoder_Init(void);
void Encoder_GetCount(int32_t *countA, int32_t *countB);
float Encoder_GetSpeedA(float dt_ms);
float Encoder_GetSpeedB(float dt_ms);

extern volatile int16_t dirA, dirB;
extern int32_t encoder_countA, encoder_countB;

#endif
