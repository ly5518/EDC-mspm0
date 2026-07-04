#ifndef _ENCODER_H
#define _ENCODER_H
#include "ti_msp_dl_config.h"

void Encoder_Init(void);
extern volatile int Get_Encoder_countA, Get_Encoder_countB;

#endif
