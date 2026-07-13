#include "board.h"
#include <stdio.h>
#include <math.h>
#include "bsp_spi.h"
#include "IMU.h"
#include "LCD.h"
#include "encoder.h"
#include "motor.h"
#include "Gray.h"
#include "Easy_Menu.h"
#include "Key.h"
#include "bsp_sg90.h"

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
int countA = 0, countB = 0;      // 编码器计数(每5ms的编码器计数)

float turned = 0;                     // 转过的角度
volatile uint8_t square_flag = 0;     // 走正方形标志

/* ============ 正方形状态机 ============ */
typedef enum { SQ_IDLE, SQ_STRAIGHT, SQ_TURN, SQ_DONE } SquareState;
volatile SquareState sq_state = SQ_IDLE;
int sq_step = 0;           // 当前第几步（0~3）
float sq_target = 600.0f; // 直走目标距离(mm)
float sq_turn_target = 90.0f; // 转弯目标角度
float sq_start_yaw = 0;
float sq_straight_yaw = 0;   // 直走时的起始yaw角

float ypr[3];

/* ============ 获取净距离（用encoder的distA_mm/distB_mm） ============ */
float Get_Traveled(void)
{
    float a = distA_mm > 0 ? distA_mm : -distA_mm;
    float b = distB_mm > 0 ? distB_mm : -distB_mm;
    return (a + b) / 2.0f;
}

/* ============ 正方形状态机（每10ms调用一次，非阻塞） ============ */
void Square_Run(void)
{
    switch (sq_state)
    {
    case SQ_IDLE:
        break;

    case SQ_STRAIGHT:
    {
        float traveled = Get_Traveled();
        if (fabs(traveled) < sq_target)
        {
            // IMU纠偏：yaw偏差→左右轮差速修正
            float yaw_error = ypr[0] - sq_straight_yaw;
            // 处理±180度边界
            if (yaw_error > 180.0f) yaw_error -= 360.0f;
            if (yaw_error < -180.0f) yaw_error += 360.0f;
            int correction = (int)(yaw_error * 100);  // 比例系数
            if (correction > 600) correction = 600;
            if (correction < -600) correction = -600;
            Set_PWM(-2000 + correction,2000 + correction);
        }
        else
        {
            Set_PWM(0, 0);          // 立刻停车
            delay_ms(100);          // 等车停稳
            IMU_getYawPitchRoll(ypr);
            sq_start_yaw = ypr[0];
            turned = 0;
            sq_state = SQ_TURN;    // 切换到转弯
        }
        break;
    }

    case SQ_TURN:
        if (turned < sq_turn_target)
        {
            Set_PWM(-1000, -1000);  // 右转
        }
        else
        {
            Set_PWM(0, 0);
            sq_step++;
            if (sq_step >= 4)
            {
                sq_step = 0;
                sq_state = SQ_DONE;
            }
            else
            {
                Encoder_ResetDist();
                sq_straight_yaw = ypr[0];  // 记录转弯后的yaw角
                sq_state = SQ_STRAIGHT;
            }
        }
        break;

    case SQ_DONE:
        Set_PWM(0, 0);
        sq_state = SQ_IDLE;
        square_flag = 0;
        break;
    }
}


/* ============ Yaw 角度（带清零补偿）============ */
float yaw_angle = 0;          // 补偿后的 yaw 角（°），可直接使用
static float yaw_offset = 0;  // 清零时记录的原始 yaw 值
No_MCU_Sensor gray_sensor;     // 灰度传感器结构体
unsigned short gray_analog[8]; // 原始模拟量缓存
uint16_t gray_white_ref[8];    // 白场参考值
uint16_t gray_black_ref[8];    // 黑场参考值
uint16_t gray_threshold[8];    // 灰度阈值
char gray_digital_str[9];      // 灰度二进制字符串 "10101001"

/* ================================================================ 校准状态 ================================================================ */
#define CAL_STATE_IDLE 0
#define CAL_STATE_WHITE 1
#define CAL_STATE_BLACK 2
#define CAL_SAMPLE_NUM_USER 50 // 用户校准采样次数

static uint8_t cal_state = CAL_STATE_IDLE;
static uint8_t cal_sample_cnt = 0;
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
            gray_threshold[i] = 2000; // 未校准的默认值
    }
}

/* 白场校准启动（由菜单 Switch 回调调用）*/
void Gray_White_Cal_Start(void)
{
    for (int i = 0; i < 8; i++)
        cal_sum[i] = 0;
    cal_sample_cnt = 0;
    cal_state = CAL_STATE_WHITE;
}

/* 黑场校准启动（由菜单 Switch 回调调用）*/
void Gray_Black_Cal_Start(void)
{
    for (int i = 0; i < 8; i++)
        cal_sum[i] = 0;
    cal_sample_cnt = 0;
    cal_state = CAL_STATE_BLACK;
}

/* 校准增量采集（在主循环 100ms 周期中调用，每次采集 1 个样本）*/
static void Gray_Cal_Collect_Incremental(void)
{
    if (cal_state == CAL_STATE_IDLE)
        return;

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

/* ================================================================ Yaw 清零 ================================================================ */
void Yaw_Zero(void)
{
    yaw_offset = ypr[0];
    yaw_angle = 0;
}

/* ================================================================ 主函数 ================================================================ */
int main(void)
{
    // int i = 0;

    // SYSCFG_DL_init();

    // Set_Servo_Angle(0);
    // delay_ms(1000);
    // Set_Servo_Angle(45);
    // delay_ms(1000);
    // Set_Servo_Angle(90);
    // delay_ms(1000);
    // Set_Servo_Angle(135);
    // delay_ms(1000);
    // Set_Servo_Angle(180);
    // delay_ms(1000);
    // Set_Servo_Angle(135);
    // delay_ms(1000);
    // Set_Servo_Angle(90);
    // delay_ms(1000);
    // Set_Servo_Angle(45);
    // delay_ms(1000);
    // Set_Servo_Angle(0);
    // delay_ms(1000);
    // while(1)
    // {
    //     // i: 0→180→0，映射成角度
    //     int angle;
    //     if(i <= 180)
    //         angle = i;
    //     else
    //         angle = 360 - i;

    //     Set_Servo_Angle(angle);
    //     delay_ms(10);

    //     i++;
    //     if(i >= 360) i = 0;
    // }
    


    board_init();
    delay_ms(100);

    IMU_init();
    TimeA1_Init();
    LCD_init();
    Set_Servo_Angle(0);
    LCD_set_color(LCD_DEFAULT_PENCOLOR, LCD_DEFAULT_BGCOLOR);

    // 初始化 Easy_Menu，绑定 LCD 显示回调（字体 6x8，与 Easy_Menu CHAR_WIDTH=6, CHAR_HEIGHT=8 匹配）
    Easy_Menu_Init(LCD_Display_Char,
                   LCD_Display_Char_Line,
                   LCD_Display_Chinese_Char,
                   LCD_Display_Chinese_Char_Line);

    Motor_Init();
    Encoder_Init();
    Motor_PID_Init();

    /* ADC 中断使能（轮询方式需要读取中断状态寄存器）*/
    DL_ADC12_clearInterruptStatus(ADC12_0_INST, DL_ADC12_INTERRUPT_MEM0_RESULT_LOADED);
    DL_ADC12_enableInterrupt(ADC12_0_INST, DL_ADC12_INTERRUPT_MEM0_RESULT_LOADED);

    No_MCU_Ganv_Sensor_Init_Frist(&gray_sensor); // 灰度传感器首次初始化
    delay_ms(20);

    /* 呼吸灯变量 */
    int breath_pwm = 0;
    int breath_dir = 1;
    int breath_cnt = 0;

    /* 定距控制变量 */
    #define TARGET_DIST_MM  2000   // 目标距离(mm)
    #define DRIVE_PWM       2000  // 驱动PWM
    uint8_t dist_mode = 0;        // 1=定距模式, 0=停止（已由状态机控制）
uint8_t reset_flag = 0;       // 串口发'r'重置距离
    uint8_t dist_done = 0;

    while (1)
    {
        // 菜单系统周期刷新（传入 menu_count ，单位 1ms）
        Easy_Menu_Display(menu_count);

        if (_5ms_flag)
        {
            _5ms_flag = 0;

            IMU_getYawPitchRoll(ypr);
            if (sq_state == SQ_TURN)
                turned = fabs(ypr[0] - sq_start_yaw);

            // 更新补偿后的 yaw_angle，保持在 [-180, 180]
            yaw_angle = ypr[0] - yaw_offset;
            if (yaw_angle > 180.0f)  yaw_angle -= 360.0f;
            if (yaw_angle < -180.0f) yaw_angle += 360.0f;
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

        if (_10ms_flag)
        {
            _10ms_flag = 0;
            Square_Run();  // 正方形状态机（非阻塞）
        }

        // 调试打印（每100ms）
        {
            static int dbg_cnt = 0;
            if (++dbg_cnt >= 10) { dbg_cnt = 0; printf("state:%d yaw:%.1f turned:%.1f target:%.1f dist:%.1f\n", sq_state, ypr[0], turned, sq_turn_target, Get_Traveled()); }
        }

        // 菜单触发：启动正方形状态机
        if (square_flag && sq_state == SQ_IDLE)
        {
            Velocity_Reset();
            Encoder_ResetDist();
            sq_straight_yaw = ypr[0];  // 记录起始yaw角
            sq_step = 0;
            sq_state = SQ_STRAIGHT;
        }
            
            // 编码器速度计算
            Encoder_CalcSpeed();

            // // PID 闭环控制（速度环 + 转向环）—— 暂时禁用
            // Motor_PID_Calc();

            /* ---- 呼吸灯双电机 ---- */
            // breath_cnt++;
            // if (breath_cnt >= 10) // 每100ms变一次
            // {
            //     breath_cnt = 0;
            //     breath_pwm += breath_dir * 100;
            //     if (breath_pwm >= 8000) { breath_pwm = 8000; breath_dir = -1; }
            //     if (breath_pwm <= 0)    { breath_pwm = 0;    breath_dir = 1;  }
            // }
            /* ---- 定距控制 ---- */
            // 定距控制已由状态机处理
            // if (dist_mode && !dist_done)
            // {
            //     int32_t absA = distA_mm > 0 ? distA_mm : -distA_mm;
            //     int32_t absB = distB_mm > 0 ? distB_mm : -distB_mm;
            //     if (absA < TARGET_DIST_MM && absB < TARGET_DIST_MM)
            //     {
            //         Set_PWM(-DRIVE_PWM, DRIVE_PWM);
            //     }
            //     else
            //     {
            //         Set_PWM(0, 0);
            //         dist_done = 1;
            //     }
            // }
            // else
            // {
            //     Set_PWM(0, 0);
            // }

            printf("cntA:%u distA:%d L:%d done:%d\r\n",
                   debug_cntA, (int)distA_mm, speedA, dist_done);
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
        if (count == 100) // 100 x 100us = 10ms
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

// GPIO中断处理（编码器方向 + 按键 + NRF24L01）
void GROUP1_IRQHandler(void)
{
    // ---- GPIOA: 编码器A方向 + NRF24L01 IRQ ----
    uint32_t statusA = DL_GPIO_getEnabledInterruptStatus(GPIOA,
        ENCODER1_DIR_PIN_ENCODER1_PIN | NRF24L01_IRQ_PIN);

    if (statusA & ENCODER1_DIR_PIN_ENCODER1_PIN)
    {
        Encoder_UpdateDir();
        DL_GPIO_clearInterruptStatus(GPIOA, ENCODER1_DIR_PIN_ENCODER1_PIN);
    }
    if (statusA & NRF24L01_IRQ_PIN)
    {
        DL_GPIO_clearInterruptStatus(GPIOA, NRF24L01_IRQ_PIN);
    }

    // ---- GPIOB: 编码器B方向 + 按键 ----
    uint32_t statusB = DL_GPIO_getEnabledInterruptStatus(GPIOB,
        ENCODER2_DIR_PIN_ENCODER2_PIN | KEY_up_PIN);

    if (statusB & ENCODER2_DIR_PIN_ENCODER2_PIN)
    {
        Encoder_UpdateDir();
        DL_GPIO_clearInterruptStatus(GPIOB, ENCODER2_DIR_PIN_ENCODER2_PIN);
    }
    if (statusB & KEY_up_PIN)
    {
        DL_GPIO_clearInterruptStatus(GPIOB, KEY_up_PIN);
    }
}

void TimeA1_Init(void)
{
    // 设置中断优先级：Timer=0(最高), GPIO=1(较低)
    // 确保编码器采样ISR不被GPIO中断打断，消除竞态条件
    NVIC_SetPriority(TIMER_1_INST_INT_IRQN, 0);
    NVIC_SetPriority(GPIO_MULTIPLE_GPIOA_INT_IRQN, 1);
    NVIC_SetPriority(GPIO_MULTIPLE_GPIOB_INT_IRQN, 1);

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
