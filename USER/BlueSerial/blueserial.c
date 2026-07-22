/**
 * @file    blueserial.c
 * @brief   蓝牙串口：发送yaw给云台
 */

#include "blueserial.h"
#include <stdio.h>

#define BT_UART  UART_blueserial_INST

void BlueSerial_Init(void)
{
    DL_UART_Main_enable(BT_UART);
    printf("BT_INIT\r\n");  // 通过调试串口确认初始化执行了
    BlueSerial_SendString("hello\r\n");
}

void BlueSerial_SendChar(char ch)
{
    DL_UART_Main_transmitData(BT_UART, (uint8_t)ch);
    while (!DL_UART_Main_isTXFIFOEmpty(BT_UART));
}

void BlueSerial_SendString(const char *str)
{
    while (*str)
        BlueSerial_SendChar(*str++);
}

void BlueSerial_SendYaw(float yaw)
{
    // 整数+小数分开算，避免snprintf不支持浮点
    int whole = (int)yaw;
    int frac = (int)((yaw - whole) * 100);
    if (frac < 0) frac = -frac;
    char buf[32];
    snprintf(buf, sizeof(buf), "[yaw,%d.%02d]\r\n", whole, frac);
    BlueSerial_SendString(buf);
}
