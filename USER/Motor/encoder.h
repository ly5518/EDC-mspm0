#ifndef _ENCODER_H
#define _ENCODER_H
#include "ti_msp_dl_config.h"

void Encoder_Init(void);
float Encoder_GetSpeedA(float dt_ms);
float Encoder_GetSpeedB(float dt_ms);
extern volatile int Get_Encoder_countA, Get_Encoder_countB;

#endif
