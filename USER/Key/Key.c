#include "Key.h"

static uint8_t Key_Num;

void Key_Init(void)
{
    // 按键已在 SysConfig 中配置为上拉输入，无需额外初始化
}

uint8_t Key_GetNum(void)
{
    uint8_t Temp;
    if (Key_Num)
    {
        Temp = Key_Num;
        Key_Num = 0;
        return Temp;
    }
    return 0;
}

static uint8_t Key_GetState(void)
{
    if (DL_GPIO_readPins(KEY_PORT, KEY_up_PIN) == 0)
        return 1;
    if (DL_GPIO_readPins(KEY_PORT, KEY_down_PIN) == 0)
        return 2;
    if (DL_GPIO_readPins(KEY_PORT, KEY_left_PIN) == 0)
        return 3;
    if (DL_GPIO_readPins(KEY_PORT, KEY_right_PIN) == 0)
        return 4;
    return 0;
}

void Key_Tick(void)
{
    static uint8_t Count;
    static uint8_t CurrState, PrevState;

    Count++;
    if (Count >= 20)
    {
        Count = 0;

        PrevState = CurrState;
        CurrState = Key_GetState();

        if (CurrState == 0 && PrevState != 0)
        {
            Key_Num = PrevState;
        }
    }
}
