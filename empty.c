#include "board.h"
#include <stdio.h>
#include "bsp_spi.h"
#include "IMU.h"
#include "LCD.h"
#include "encoder.h"
#include "motor.h"
#include "Gray.h"
#include "Easy_Menu.h"
#include "Key.h"

void TimeA1_Init(void);
static void LCD_Display_Char(unsigned short int x, unsigned short int y, char ch, unsigned char reverse_flag);
static void LCD_Display_Char_Line(unsigned short int x, unsigned char line, char ch, unsigned char reverse_flag);
static void LCD_Display_Chinese_Char(unsigned short int x, unsigned short int y, char *ch, unsigned char reverse_flag);
static void LCD_Display_Chinese_Char_Line(unsigned short int x, unsigned char line, char *ch, unsigned char reverse_flag);

extern uint32_t nowtime;
volatile uint32_t count = 0;
static uint32_t menu_count = 0;
volatile uint8_t _1ms_flag = 0;  // 1ms 定时标志
volatile uint8_t _5ms_flag = 0;  // 5ms 定时标志
volatile uint8_t _10ms_flag = 0; // 10ms 定时标志
volatile uint8_t print_flag = 0; // 打印标志（每 100ms）
volatile uint8_t print_cnt = 0;  // 打印计数器
int speedA = 0, speedB = 0;      // 编码器速度（每 5ms 的脉冲数）

float ypr[3];
No_MCU_Sensor gray_sensor;        // 灰度传感器结构体
unsigned short gray_analog[8];    // 原始模拟量缓存
uint16_t gray_white_ref[8]; // 白场参考值
uint16_t gray_black_ref[8]; // 黑场参考值
uint16_t gray_threshold[8]; // 灰度阈值
char gray_digital_str[9]; // 灰度二进制字符串 "10101001"

/* ================================================================ 校准状态 ================================================================ */
#define CAL_STATE_IDLE      0
#define CAL_STATE_WHITE     1
#define CAL_STATE_BLACK     2
#define CAL_SAMPLE_NUM_USER 50  // 用户校准采样次数

static uint8_t  cal_state = CAL_STATE_IDLE;
static uint8_t  cal_sample_cnt = 0;
static uint32_t cal_sum[8] = {0};

/* 通过 extern 访问 Easy_Menu_User.c 中的数据结构体 */
extern struct
{
    unsigned char LED1__data, LED2__data, LED3__data, LED4__data;
    unsigned char uint8_data;
    unsigned short int uint16_data;
    unsigned int uint32_data;
    signed char int8_data;
    signed short int int16_data;
    signed int int32_data;
    float float_data;
    unsigned char hours, minutes, seconds, millisecond;
    float yaw;
    unsigned short int gray_binary;
    float pitch, roll;
} Easy_Menu_Ui_Data;

/* ================================================================ 校准函数 ================================================================ */
#define CAL_SAMPLE_NUM 100 // 校准采样次数

extern void Get_Analog_value(unsigned short *result);

/* 采集 N 次模拟量并求平均 */
static void Gray_Collect_Average(unsigned short *result)
{
    unsigned int sum[8] = {0};
    unsigned short buf[8];
    for (int n = 0; n < CAL_SAMPLE_NUM; n++)
    {
        Get_Analog_value(buf);
        for (int i = 0; i < 8; i++)
            sum[i] += buf[i];
        delay_us(500);
    }
    for (int i = 0; i < 8; i++)
        result[i] = sum[i] / CAL_SAMPLE_NUM;
}

/* 计算阈值：靠黑色端 1/3 等分点 = (白 + 2×黑) / 3 */
static void Gray_Calc_Threshold(void)
{
    for (int i = 0; i < 8; i++)
    {
        if (gray_white_ref[i] > 0 && gray_black_ref[i] > 0)
            gray_threshold[i] = (gray_white_ref[i] + 2 * gray_black_ref[i]) / 3;
        else
            gray_threshold[i] = 2000;  // 未校准的默认值
    }
}

/* 白场校准启动（由菜单 Switch 回调调用）*/
void Gray_White_Cal_Start(void)
{
    for (int i = 0; i < 8; i++) cal_sum[i] = 0;
    cal_sample_cnt = 0;
    cal_state = CAL_STATE_WHITE;
}

/* 黑场校准启动（由菜单 Switch 回调调用）*/
void Gray_Black_Cal_Start(void)
{
    for (int i = 0; i < 8; i++) cal_sum[i] = 0;
    cal_sample_cnt = 0;
    cal_state = CAL_STATE_BLACK;
}

/* 校准增量采集（在主循环 100ms 周期中调用，每次采集 1 个样本）*/
static void Gray_Cal_Collect_Incremental(void)
{
    if (cal_state == CAL_STATE_IDLE) return;

    unsigned short buf[8];
    Get_Analog_value(buf);
    for (int i = 0; i < 8; i++)
        cal_sum[i] += buf[i];
    cal_sample_cnt++;

    if (cal_sample_cnt >= CAL_SAMPLE_NUM_USER)
    {
        uint16_t *ref = (cal_state == CAL_STATE_WHITE) ? gray_white_ref : gray_black_ref;
        for (int i = 0; i < 8; i++)
            ref[i] = cal_sum[i] / CAL_SAMPLE_NUM_USER;
        Gray_Calc_Threshold();
        cal_state = CAL_STATE_IDLE;
    }
}

/* ================================================================ 主函数 ================================================================ */
int main(void)
{
    board_init();
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

    /* ADC 中断使能（轮询方式需要读取中断状态寄存器）*/
    DL_ADC12_clearInterruptStatus(ADC12_0_INST, DL_ADC12_INTERRUPT_MEM0_RESULT_LOADED);
    DL_ADC12_enableInterrupt(ADC12_0_INST, DL_ADC12_INTERRUPT_MEM0_RESULT_LOADED);

    No_MCU_Ganv_Sensor_Init_Frist(&gray_sensor); // 灰度传感器首次初始化
    delay_ms(20);

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
            case 1:
                Easy_Menu_Input(EASY_MENU_UP);
                break;
            case 2:
                Easy_Menu_Input(EASY_MENU_DOWN);
                break;
            case 3:
                Easy_Menu_Input(EASY_MENU_LEFT);
                break;
            case 4:
                Easy_Menu_Input(EASY_MENU_RIGHT);
                break;
            default:
                Easy_Menu_Input(EASY_MENU_NONE);
                break;
            }
        }

        // 菜单系统周期刷新（传入 menu_count ，单位 1ms）
        Easy_Menu_Display(menu_count);

        // 每 100ms 采集灰度并打印模拟量
        if (print_flag)
        {
            print_flag = 0;

            No_Mcu_Ganv_Sensor_Task_Without_tick(&gray_sensor); // 采集灰度
            Get_Anolog_Value(&gray_sensor, gray_analog);        // 取原始模拟量

            // 校准增量采集（非阻塞，每次 1 个样本）
            Gray_Cal_Collect_Incremental();

            // 使用校准阈值进行二值化，直接生成二进制字符串（MSB 在左：bit7 bit6 ... bit0）
            for (int i = 0; i < 8; i++)
            {
                unsigned short th = (gray_threshold[i] > 0) ? gray_threshold[i] : 2000;
                gray_digital_str[7 - i] = (gray_analog[i] > th) ? '1' : '0';
            }
            gray_digital_str[8] = '\0';

            Easy_Menu_Display_Refresh(); // 刷新当前页面（更新 Text_Item 显示）
        }
    }
}

// TIMER_1 every 100us
void TIMER_1_INST_IRQHandler(void)
{
    switch (DL_TimerA_getPendingInterrupt(TIMER_1_INST))
    {
    case DL_TIMER_IIDX_ZERO:
        count++;
        nowtime++;
        if ((count % 10) == 0) // 10 x 100us = 1ms
        {
            _1ms_flag = 1;
            menu_count++;
            Key_Tick();
        }
        if ((count % 50) == 0) // 50 x 100us = 5ms
        {
            _5ms_flag = 1;
            print_cnt++;
            if (print_cnt >= 20) // 20 x 5ms = 100ms
            {
                print_cnt = 0;
                print_flag = 1;
            }
        }
        if ((count % 100) == 0) // 100 x 100us = 10ms
        {
            count = 0;
            _10ms_flag = 1;
        }
        DL_TimerG_clearInterruptStatus(TIMER_1_INST, DL_TIMER_IIDX_ZERO);
        break;
    default:
        break;
    }
}

void TimeA1_Init(void)
{
    NVIC_ClearPendingIRQ(TIMER_1_INST_INT_IRQN);
    NVIC_EnableIRQ(TIMER_1_INST_INT_IRQN);
}

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
    (void)x;
    (void)y;
    (void)ch;
    (void)reverse_flag;
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
    (void)x;
    (void)line;
    (void)ch;
    (void)reverse_flag;
}
