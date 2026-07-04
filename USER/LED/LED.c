#include "LED.h"

void LED_On(void)
{
    DL_GPIO_clearPins(LED_PORT, LED_B22_PIN);  // 低电平点亮
}

void LED_Off(void)
{
    DL_GPIO_setPins(LED_PORT, LED_B22_PIN);    // 高电平熄灭
}

void LED_Toggle(void)
{
    DL_GPIO_togglePins(LED_PORT, LED_B22_PIN);
}

void LED_Set(uint8_t state)
{
    if (state)
        LED_On();
    else
        LED_Off();
}
