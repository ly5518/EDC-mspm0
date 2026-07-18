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

    BlueSerial_SendString("BT_OK\r\n");
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
    char buf[32];
    snprintf(buf, sizeof(buf), "[yaw,%.2f]\r\n", yaw);
    BlueSerial_SendString(buf);
}
