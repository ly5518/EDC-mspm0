/**
 * @file    blueserial.c
 * @brief   蓝牙串口：发送yaw给云台，接收并解析 [ERR,x,y] 协议
 */

#include "blueserial.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BT_UART  UART_blueserial_INST

/* ============ 蓝牙接收解析 ============ */
#define BT_RX_BUF_SIZE 64

static char     bt_rx_buf[BT_RX_BUF_SIZE];
static uint8_t  bt_rx_idx = 0;
static uint8_t  bt_rx_state = 0;

volatile int16_t bt_err_x = 0;
volatile int16_t bt_err_y = 0;
volatile uint8_t bt_err_flag = 0;
char bt_rx_raw[64] = {0};

static uint8_t BlueSerial_ParseERR(const char *buf)
{
    if (strncmp(buf, "ERR,", 4) != 0) return 0;
    const char *p = &buf[4];
    char *end;
    long x = strtol(p, &end, 10);
    if (end == p || *end != ',') return 0;
    p = end + 1;
    long y = strtol(p, &end, 10);
    if (end == p || *end != '\0') return 0;
    bt_err_x = (int16_t)x;
    bt_err_y = (int16_t)y;
    bt_err_flag = 1;
    return 1;
}

void BlueSerial_RxProcess(void)
{
    uint8_t ch = (uint8_t)DL_UART_Main_receiveData(BT_UART);
    if (bt_rx_state == 0)
    {
        if (ch == '[') { bt_rx_state = 1; bt_rx_idx = 0; }
        return;
    }
    if (ch == '[') { bt_rx_idx = 0; return; }
    if (ch == ']')
    {
        bt_rx_buf[bt_rx_idx] = '\0';
        strncpy(bt_rx_raw, bt_rx_buf, sizeof(bt_rx_raw) - 1);
        bt_rx_raw[sizeof(bt_rx_raw) - 1] = '\0';
        BlueSerial_ParseERR(bt_rx_buf);
        bt_rx_state = 0;
        bt_rx_idx = 0;
        return;
    }
    if (bt_rx_idx < BT_RX_BUF_SIZE - 1)
        bt_rx_buf[bt_rx_idx++] = (char)ch;
    else
        { bt_rx_state = 0; bt_rx_idx = 0; }
}

void BlueSerial_Init(void)
{
    DL_UART_Main_enable(BT_UART);
    NVIC_ClearPendingIRQ(UART_blueserial_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_blueserial_INST_INT_IRQN);
    printf("BT_INIT\r\n");
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
    int whole = (int)yaw;
    int frac = (int)((yaw - whole) * 100);
    if (frac < 0) frac = -frac;
    char buf[32];
    snprintf(buf, sizeof(buf), "[yaw,%d.%02d]\r\n", whole, frac);
    BlueSerial_SendString(buf);
}

int16_t BlueSerial_GetErrX(void) { return bt_err_x; }
int16_t BlueSerial_GetErrY(void) { return bt_err_y; }
uint8_t BlueSerial_HasErr(void) { return bt_err_flag; }
void BlueSerial_ClearErr(void) { bt_err_flag = 0; }
const char* BlueSerial_GetRaw(void) { return bt_rx_raw; }

/* UART1 中断处理 */
void UART_blueserial_INST_IRQHandler(void)
{
    switch (DL_UART_getPendingInterrupt(UART_blueserial_INST))
    {
    case DL_UART_IIDX_RX:
        while (!DL_UART_isRXFIFOEmpty(UART_blueserial_INST))
            BlueSerial_RxProcess();
        break;
    default:
        break;
    }
}
