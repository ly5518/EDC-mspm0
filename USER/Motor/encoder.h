#ifndef _ENCODER_H
#define _ENCODER_H
#include "ti_msp_dl_config.h"

void Encoder_Init(void);
void Encoder_Sample(void);

extern volatile float speedA, speedB;

#endif
