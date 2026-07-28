#ifndef __BLUESERIAL_H__
#define __BLUESERIAL_H__

#include "ti_msp_dl_config.h"
#include <stdint.h>

void BlueSerial_Init(void);
void BlueSerial_SendChar(char ch);
void BlueSerial_SendString(const char *str);
void BlueSerial_SendYaw(float yaw);

int16_t BlueSerial_GetErrX(void);
int16_t BlueSerial_GetErrY(void);
uint8_t BlueSerial_HasErr(void);
void BlueSerial_ClearErr(void);
const char* BlueSerial_GetRaw(void);

#endif
