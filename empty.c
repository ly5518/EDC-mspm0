#include "board.h"
#include <stdio.h>
#include "bsp_spi.h"
#include "IMU.h"
#include "LCD.h"
#include "encoder.h"
#include "motor.h"
#include "gray.h"
#include "Easy_Menu.h"
#include "Key.h"

// IIC: PA0=SDA, PA1=SCL
// SPI: PB6=CS, PB7=MISO, PB8=MOSI, PB9=SCLK

void TimeA1_Init(void);
extern uint32_t nowtime;

volatile uint32_t count = 0;
volatile uint8_t key_count = 0;    // 按键扫描计数器
volatile uint8_t _5ms_flag = 0;
volatile uint8_t print_flag = 0;   // 打印标志（每 100ms）
volatile uint8_t print_cnt = 0;    // 打印计数器
int speedA = 0, speedB = 0;        // 编码器速度（每 5ms 的脉冲数）

float ypr[3];

/* ================================================================ LCD 回调函数（供 Easy_Menu 使用） ================================================================ */

/**
 * @brief  LCD 显示单个 ASCII 字符（像素坐标）
 * @param  x: 像素 X 坐标
 * @param  y: 像素 Y 坐标
 * @param  ch: ASCII 字符
 * @param  reverse_flag: 0-正常显示, 1-反色显示
 */
static void LCD_Display_Char(unsigned short int x, unsigned short int y, char ch, unsigned char reverse_flag)
{
    if (reverse_flag)
        LCD_set_color(RGB565_BLACK, RGB565_WHITE);
    LCD_show_char(x, y, ch);
    if (reverse_flag)
        LCD_set_color(LCD_DEFAULT_PENCOLOR, LCD_DEFAULT_BGCOLOR);
}

/**
 * @brief  LCD 显示单个 ASCII 字符（行坐标，Y 以行为单位）
 * @param  x: 像素 X 坐标
 * @param  line: 行号（0 ~ SCREEN_HEIGHT/CHAR_HEIGHT - 1）
 * @param  ch: ASCII 字符
 * @param  reverse_flag: 0-正常显示, 1-反色显示
 */
static void LCD_Display_Char_Line(unsigned short int x, unsigned char line, char ch, unsigned char reverse_flag)
{
    LCD_Display_Char(x, line * CHAR_HEIGHT, ch, reverse_flag);
}

/**
 * @brief  LCD 显示中文字符（像素坐标，当前不支持）
 * @param  x: 像素 X 坐标
 * @param  y: 像素 Y 坐标
 * @param  ch: 中文字符指针（GB2312 编码，2 字节）
 * @param  reverse_flag: 0-正常显示, 1-反色显示
 */
static void LCD_Display_Chinese_Char(unsigned short int x, unsigned short int y, char *ch, unsigned char reverse_flag)
{
    // 当前 LCD 驱动不支持中文字库，留作扩展
    (void)x; (void)y; (void)ch; (void)reverse_flag;
}

/**
 * @brief  LCD 显示中文字符（行坐标，当前不支持）
 * @param  x: 像素 X 坐标
 * @param  line: 行号
 * @param  ch: 中文字符指针（GB2312 编码，2 字节）
 * @param  reverse_flag: 0-正常显示, 1-反色显示
 */
static void LCD_Display_Chinese_Char_Line(unsigned short int x, unsigned char line, char *ch, unsigned char reverse_flag)
{
    // 当前 LCD 驱动不支持中文字库，留作扩展
    (void)x; (void)line; (void)ch; (void)reverse_flag;
}

/* ================================================================ 主函数 ================================================================ */
int main(void)
{
    board_init();
    printf("board init ok\r\n");
    delay_ms(100);

    IMU_init();
    TimeA1_Init();
    LCD_init();

    LCD_set_color(LCD_DEFAULT_PENCOLOR, LCD_DEFAULT_BGCOLOR);

    // 初始化 Easy_Menu，绑定 LCD 显示回调（字体 6x8，与 Easy_Menu CHAR_WIDTH=6, CHAR_HEIGHT=8 匹配）
    Easy_Menu_Init(LCD_Display_Char,
                   LCD_Display_Char_Line,
                   LCD_Display_Chinese_Char,
                   LCD_Display_Chinese_Char_Line);

    Motor_Init();
    Encoder_Init();
    // gray_init();
    delay_ms(20);
    printf("enter loop\r\n");

    Set_PWM(2000, -2000);
    while (1)
    {
        if (_5ms_flag)
        {
            _5ms_flag = 0;

            // 读取编码器速度并清零（每 5ms 一次）
            speedA = Get_Encoder_countA;
            speedB = Get_Encoder_countB;
            Get_Encoder_countA = 0;
            Get_Encoder_countB = 0;

            IMU_getYawPitchRoll(ypr);
        }

        // 按键扫描 → 菜单输入
        {
            uint8_t key = Key_GetNum();
            switch (key)
            {
                case 1: Easy_Menu_Input(EASY_MENU_UP);    break;
                case 2: Easy_Menu_Input(EASY_MENU_DOWN);  break;
                case 3: Easy_Menu_Input(EASY_MENU_LEFT);  break;
                case 4: Easy_Menu_Input(EASY_MENU_RIGHT); break;
                default: Easy_Menu_Input(EASY_MENU_NONE); break;
            }
        }

        // 菜单系统周期刷新（传入 nowtime 作为 tick，单位 100us）
        Easy_Menu_Display(nowtime);

        // 每 100ms 打印一次调试信息
        // if (print_flag)
        // {
        //     print_flag = 0;
        //     printf("YPR:%.1f,%.1f,%.1f  SPD:%d,%d\r\n",
        //            ypr[0], ypr[1], ypr[2], speedA, speedB);
        //     LCDprint(0, 0, "Y:%.1f P:%.1f R:%.1f  ", ypr[0], ypr[1], ypr[2]);
        //     LCDprint(0, 1*8, "SA:%d SB:%d    ", speedA, speedB);
        // }
    }
}

void TimeA1_Init(void)
{
    NVIC_ClearPendingIRQ(TIMER_1_INST_INT_IRQN);
    NVIC_EnableIRQ(TIMER_1_INST_INT_IRQN);
}

// TIMER_1 every 100us
void TIMER_1_INST_IRQHandler(void)
{
    switch (DL_TimerA_getPendingInterrupt(TIMER_1_INST))
    {
    case DL_TIMER_IIDX_ZERO:
        nowtime++;
        count++;
        key_count++;
        if (key_count >= 10) // 10 x 100us = 1ms
        {
            key_count = 0;
            Key_Tick();
        }
        if (count >= 50) // 50 x 100us = 5ms
        {
            count = 0;
            _5ms_flag = 1;
            print_cnt++;
            if (print_cnt >= 20) // 20 x 5ms = 100ms
            {
                print_cnt = 0;
                print_flag = 1;
            }
        }
        DL_TimerG_clearInterruptStatus(TIMER_1_INST, DL_TIMER_IIDX_ZERO);
        break;
    default:
        break;
    }
}
