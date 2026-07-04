#ifndef __LED_H
#define __LED_H

#include "ti_msp_dl_config.h"

void LED_On(void);
void LED_Off(void);
void LED_Toggle(void);
void LED_Set(uint8_t state);

#endif
