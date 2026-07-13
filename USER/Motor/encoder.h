#ifndef _ENCODER_H
#define _ENCODER_H
#include "ti_msp_dl_config.h"

void Encoder_Init(void);
void Encoder_Sample(void);
void Encoder_CalcSpeed(void);
void Encoder_UpdateDir(void);

extern volatile int16_t speedA, speedB;
extern volatile int8_t encoder_dirA, encoder_dirB;

#endif
