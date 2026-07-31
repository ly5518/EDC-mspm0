/**
 * @file    task.c
 * @brief   任务架构：PID + 正方形任务
 */

#include "task.h"
#include <math.h>
#include <stdio.h>
#include "board.h"
#include "Easy_Menu_User.h"
extern float yaw_angle;

/* ============ PID 实例 ============ */
PID_t pid_speed = {
    .Kp = 8.8f, .Ki = 0.1f, .Kd = 0.0f,
    .OutMax = 7000, .OutMin = -7000,
    .IntegralMax = 5000, .IntegralMin = -5000,
    .Mode = PID_INCREMENTAL, .Enabled = 1
};
PID_t pid_steer = {
    .Kp = 50.0f, .Ki = 0.0f, .Kd = 0.5f,//150.0f
    .OutMax = 500, .OutMin = -500,   //800
    .IntegralMax = 800, .IntegralMin = -800,
    .Mode = PID_DERIVATIVE_ON_MEASUREMENT, .Enabled = 1
};
PID_t pid_line = {
    .Kp = 35.0f, .Ki = 0.0f, .Kd = 0.1f,
    .OutMax = 3000, .OutMin = -3000,
    .IntegralMax = 3000, .IntegralMin = -3000,
    .Mode = PID_DERIVATIVE_ON_MEASUREMENT, .Enabled = 0
};

/* ============ 目标值 ============ */
int16_t target_speed = 0;
float   target_yaw   = 0;
int16_t target_line  = 0;

/* ============ 循迹状态 ============ */
int16_t line_pos = 0;
// 旧的8路权重数组，已注释
// static const int8_t line_weight[8] = {-40, -25, -15, -5, 5, 15, 25, 40};

// 新的4路权重数组（PA15, PA16, PA17, PA27）
static const int8_t line_weight[4] = {-20, -10, 10, 20};

/* ============ 任务状态 ============ */
TaskMode_t task_mode = TASK_IDLE;
TaskPoint_t current_point = POINT_A;
float CurrentDistance = 0.0f;

/* ============ 正方形参数 ============ */
float sq_target = 600.0f;
float sq_turn_target = 90.0f;
int16_t sq_target_speed = 350;   //600;

/* ============ 转弯辅助 ============ */
static float turned = 0;
static float prev_yaw = 0;
static float turn_target = 0;
static float sq_straight_yaw = 0;
static uint8_t straight_init = 0;
uint8_t sq_cycle_count = 0;
volatile uint8_t postrack_flag = 0;
volatile uint8_t task1_flag = 0;
volatile uint8_t task2_flag = 0;
volatile uint8_t task3_flag = 0;
volatile uint8_t task4_flag = 0;
volatile uint8_t task5_flag = 0;

/* ============ 任务计时器 ============ */
volatile uint32_t task_elapsed_ms = 0;
volatile uint8_t  task_timer_running = 0;
static uint32_t task_start_tick = 0;

void Task_Timer_Start(void)
{
    task_start_tick = menu_count;
    task_elapsed_ms = 0;
    task_timer_running = 1;
}

void Task_Timer_Stop(void)
{
    task_timer_running = 0;
}

/* ============ 循迹正方形参数 ============ */
uint8_t ls_lap_target = 1;       // 目标圈数
static uint8_t ls_lap_count = 0; // 已走圈数
static float ls_yaw_sum = 0;     // IMU累计角度
static float ls_last_yaw = 0;    // 上一次yaw

/* ================================================================ */
void Task_Init(void)
{
    Set_PWM(0, 0);  // 切换任务时先停电机
    PID_Clear(&pid_speed);
    PID_Clear(&pid_steer);
    PID_Clear(&pid_line);
    Encoder_ResetDist();
    CurrentDistance = 0;
    current_point = POINT_A;
    straight_init = 0;
    sq_cycle_count = 0;
}

void Motor_PID_Sync(void)
{
    pid_speed.Kp = speed_Kp;  pid_speed.Ki = speed_Ki;  pid_speed.Kd = speed_Kd;
    pid_steer.Kp = angle_Kp;  pid_steer.Ki = angle_Ki;  pid_steer.Kd = angle_Kd;
    pid_line.Kp = line_Kp;    pid_line.Ki = line_Ki;    pid_line.Kd = line_Kd;
}

void Line_CalcPosition(void)
{
    extern char gray_digital_str[5];  // 4路数字灰度传感器 + '\0'
    int32_t sum = 0;
    uint8_t count = 0;

    // 遍历4路传感器（PA15, PA16, PA17, PA27）
    // 检测到黑线='0'，检测到白底='1'
    for (int i = 0; i < 4; i++)
    {
        if (gray_digital_str[i] == '0')  // 黑线
        {
            sum += line_weight[i];
            count++;
        }
    }
    line_pos = (count > 0) ? (int16_t)(sum / count) : 0;
}

/* ============ PID 计算 ============ */
static void PID_Calc(void)
{
    if (pid_steer.Enabled)
    {
        PID_SetTarget(&pid_steer, target_yaw);
        PID_UpdateAngle(&pid_steer, yaw_angle);
    }

    float ave_speed = ((float)speedA + (float)speedB) / 2.0f;
    if (pid_speed.Enabled)
    {
        PID_SetTarget(&pid_speed, (float)target_speed);
        PID_Update(&pid_speed, ave_speed);
    }

    if (pid_line.Enabled)
    {
        PID_SetTarget(&pid_line, (float)target_line);
        PID_Update(&pid_line, (float)line_pos);
    }

    int pwm = (int)pid_speed.Out;
    int steer = -(int)pid_steer.Out;
    int line = (int)pid_line.Out;
    int LeftPWM  = (pwm - steer + line);
    int RightPWM = (pwm + steer - line);
    LeftPWM  = (LeftPWM  > PWM_MAX) ? PWM_MAX : (LeftPWM  < PWM_MIN ? PWM_MIN : LeftPWM);
    RightPWM = (RightPWM > PWM_MAX) ? PWM_MAX : (RightPWM < PWM_MIN ? PWM_MIN : RightPWM);
    Set_PWM(LeftPWM, RightPWM);
}

/* ============ 任务调度 ============ */
void Task_Run(void)
{
    /* 更新任务计时器 */
    if (task_timer_running)
        task_elapsed_ms = menu_count - task_start_tick;

    switch (task_mode)
    {
    case TASK_IDLE:
        Set_PWM(0, 0);
        Task_Timer_Stop();
        break;
    case TASK_SQUARE:
        Task_Square();
        break;
    case TASK_LINE_SQUARE:
        Task_LineSquare();
        break;
    case TASK_LEFT_TURN:
        Task_LeftTurn();
        break;
        case TASK_1:
            Task1();
            break;
        case TASK_2: 
            Task2();
            break;
        case TASK_3:
            Task3();
            break;
        case TASK_4:
            Task4();
            break;
        case TASK_5:
            Task5();
            break;
    }
}

/* ============ 正方形任务 ============ */
void Task_Square_Start(void)
{
    task_mode = TASK_SQUARE;
    current_point = POINT_A;
    Task_Init();
    pid_speed.Enabled = 1;
    pid_steer.Enabled = 1;
    pid_line.Enabled = 0;
}

void Task_Square(void)
{
    CurrentDistance = ((float)distA_mm + (float)distB_mm) / 2.0f;

    switch (current_point)
    {
    /* ---- 直走 ---- */
    case POINT_A:
    {
        if (!straight_init)
        {
            PID_Clear(&pid_steer);
            sq_straight_yaw = yaw_angle;
            straight_init = 1;
        }

        target_speed = sq_target_speed;
        target_yaw = sq_straight_yaw;
        PID_Calc();

        if (CurrentDistance >= sq_target)
        {
            prev_yaw = yaw_angle;
            turned = 0;
            PID_Clear(&pid_steer);
            turn_target = yaw_angle - sq_turn_target;
            while (turn_target - yaw_angle > 180.0f)  turn_target -= 360.0f;
            while (turn_target - yaw_angle < -180.0f) turn_target += 360.0f;
            straight_init = 0;
            // 不清距离，转弯结束后再清
            current_point = POINT_B;
        }
        break;
    }

    /* ---- 转弯 ---- */
    case POINT_B:
    {
        float delta = yaw_angle - prev_yaw;
        if (delta > 180.0f)  delta -= 360.0f;
        if (delta < -180.0f) delta += 360.0f;
        turned += delta;
        prev_yaw = yaw_angle;

        target_speed = sq_target_speed / 2;
        target_yaw = turn_target;
        PID_Calc();

        if (fabs(turned) >= sq_turn_target - 1.0f)
        {
            sq_straight_yaw = yaw_angle;
            pid_steer.ErrorInt = 0;
            Encoder_ResetDist();
            CurrentDistance = 0;
            sq_cycle_count++;

            if (sq_cycle_count >= 4)
            {
                task_mode = TASK_IDLE;
            }
            else
            {
                current_point = POINT_A;
            }
        }
        break;
    }

    default:
        break;
    }
}

/* ================================================================ */
/*                     任务2：循迹正方形                              */
/* ================================================================ */

/* 循迹状态 */
typedef enum { LF_FOLLOW, LF_TURNING } LFState_t;
static LFState_t lf_state = LF_FOLLOW;
static float lf_turn_start = 0;
static uint8_t lf_corner_count = 0;  // 当前圈已转角数
static uint8_t lf_lap_count = 0;     // 已完成圈数

void Task_LineSquare(void)
{
    extern char gray_digital_str[5];  // 4路数字灰度传感器 + '\0'

    if (lf_state == LF_FOLLOW)
    {
        pid_speed.Enabled = 1;
        pid_line.Enabled = 1;
        pid_steer.Enabled = 0;
        target_speed = sq_target_speed;
        target_line = 0;
        PID_Calc();

        // 检测到全部黑线（4路都是'0'）→ 开始转弯
        if (gray_digital_str[0] == '0' && gray_digital_str[1] == '0' &&
            gray_digital_str[2] == '0' && gray_digital_str[3] == '0')
        {
            lf_state = LF_TURNING;
            lf_turn_start = yaw_angle;
            PID_Clear(&pid_steer);
            PID_Clear(&pid_line);
        }
    }
    else
    {
        // 转弯：保持速度，转向环转90°
        pid_speed.Enabled = 1;
        pid_line.Enabled = 0;
        pid_steer.Enabled = 1;
        target_speed = sq_target_speed;
        target_yaw = lf_turn_start - 90.0f;
        if (target_yaw < -180.0f) target_yaw += 360.0f;
        PID_Calc();

        // 转够55° 且 线重新出现 → 回到循迹
        float turned = yaw_angle - lf_turn_start;
        if (turned > 180.0f)  turned -= 360.0f;
        if (turned < -180.0f) turned += 360.0f;

        // 检测是否有任何一路检测到黑线（'0'）
        uint8_t line_found = 0;
        for (int i = 0; i < 4; i++)
            if (gray_digital_str[i] == '0') { line_found = 1; break; }

        if (fabs(turned) >= 55.0f && line_found)
        {
            lf_state = LF_FOLLOW;
            lf_corner_count++;
            PID_Clear(&pid_steer);
            PID_Clear(&pid_line);

            // 4个角 = 1圈
            if (lf_corner_count >= 4)
            {
                lf_corner_count = 0;
                lf_lap_count++;

                // 走完指定圈数 → 停车
                if (lf_lap_count >= ls_lap_target)
                {
                    lf_lap_count = 0;
                    task_mode = TASK_IDLE;
                    Set_PWM(0, 0);
                }
            }
        }
    }
}

// ---- 任务：直线循迹（测试用，跑10秒停）----
void Task_Line_Start(void)
{
    task_mode = TASK_LINE_SQUARE;
    Task_Init();
    lf_corner_count = 0;
    lf_lap_count = 0;
}


void Task_LeftTurn_Start(void)
{
    task_mode = TASK_LEFT_TURN;  
    Task_Init();
}
void Task_LeftTurn(void)
{
    Set_PWM(-300, -300);
}

/* ================================================================ */
/*           摆杆通信                                                */
/* ================================================================ */

void Pendulum_SendCmd(uint8_t cmd)
{
    DL_UART_transmitData(UART_ppr_INST, cmd);
    // while (DL_UART_isBusy(UART_ppr_INST));
}

/* ================================================================ */
/*           任务一：循迹跑一圈（A点启停线检测）                        */
/* ================================================================ */

#define LAP_RUN_SPEED        700      // 循迹速度
#define LAP_SLOW_DOWN_DIST   5800.0f  // 距离到了 5.5 米 → 减速
#define LAP_SLOW_DOWN_YAW    330.0f   // 累计角度 > 320° → 确认快走完一圈

typedef enum {
    LR_FOLLOW,      // 循迹行驶
    LR_SLOW_DOWN,   // 减速循迹
    LR_STOP         // 停车
} LapRunState_t;

static LapRunState_t lr_state = LR_FOLLOW;
static float yaw_total = 0;           // 累计航向角（0°→360°）
static float yaw_last = 0;            // 上一次 yaw

void Task1_start(void)
{
    task_mode = TASK_1;
    Task_Init();
    pid_speed.Enabled = 1;
    pid_line.Enabled  = 1;
    pid_steer.Enabled = 0;
    lr_state = LR_FOLLOW;
    yaw_total = 0;
    yaw_last = yaw_angle;
}

void Task1(void)
{
    CurrentDistance = ((float)distA_mm + (float)distB_mm) / 2.0f;

    /* 更新累计航向角（处理 ±180° 跳变） */
    float delta = yaw_angle - yaw_last;
    if (delta > 180.0f)  delta -= 360.0f;
    if (delta < -180.0f) delta += 360.0f;
    yaw_total += delta;
    yaw_last = yaw_angle;

    /* 每500ms打印距离和角度 */
    static uint32_t print_cnt = 0;
    if (++print_cnt >= 50) { print_cnt = 0; printf("dist=%.0f yaw=%.0f\r\n", CurrentDistance, yaw_total); }

    switch (lr_state)
    {
    /* ---- 正常循迹：距离 + 角度确认快走完一圈 ---- */
    case LR_FOLLOW:
    {
        target_speed = LAP_RUN_SPEED;
        target_line  = 0;
        PID_Calc();

        if (CurrentDistance >= LAP_SLOW_DOWN_DIST && yaw_total >= LAP_SLOW_DOWN_YAW)
            lr_state = LR_SLOW_DOWN;
        break;
    }

    /* ---- 减速循迹：距离到了就停车 ---- */
    case LR_SLOW_DOWN:
    {
        target_speed = LAP_RUN_SPEED / 3;
        target_line  = 0;
        PID_Calc();

        /* 距离到6米 或 角度到360° → 停车 */
        if (CurrentDistance >= 6000.0f || yaw_total >= 360.0f)
            lr_state = LR_STOP;
        break;
    }

    /* ---- 停车 ---- */
    case LR_STOP:
        Set_PWM(0, 0);
        task_mode = TASK_IDLE;
        Task_Timer_Stop();
        Stopwatch_Stop();
        break;
    default:
        break;
    }
}
void Task2_start(void)
{
    Pendulum_SendCmd(0x01);  // 通知摆杆：任务3（球从O到±5cm）
    task_mode = TASK_2;
    Task_Init();
}
void Task2(void)
{
    Set_PWM(1500,1500);
}
/* ================================================================ */
/*           任务三：AB段循迹（1.5米，≤8秒）                         */
/* ================================================================ */

#define TASK3_SPEED     600      // 循迹速度
#define TASK3_TARGET_MM 1800.0f  // AB 距离 1.5 米

void Task3_start(void)
{
    Pendulum_SendCmd(0x02);  // 通知摆杆：任务4（球稳定在O，A→B）
    task_mode = TASK_3;
    Task_Init();
    pid_speed.Enabled = 1;
    pid_line.Enabled  = 1;
    pid_steer.Enabled = 0;
}

void Task3(void)
{
    CurrentDistance = ((float)distA_mm + (float)distB_mm) / 2.0f;

    /* 梯形速度曲线：加速 → 匀速 → 减速 */
    float speed = 0;
    float base_speed  = 200.0f;    // 起步基础速度
    float accel_dist  = 500.0f;    // 加速段 500mm
    float decel_start = 1500.0f;   // 1500mm 开始减速
    float total_dist  = TASK3_TARGET_MM;

    if (CurrentDistance < accel_dist)
        speed = base_speed + (TASK3_SPEED - base_speed) * (CurrentDistance / accel_dist);
    else if (CurrentDistance < decel_start)
        speed = TASK3_SPEED;
    else
    {
        /* 进入减速段 → 停计时 */
        Stopwatch_Stop();
        if (CurrentDistance < total_dist)
            speed = TASK3_SPEED * ((total_dist - CurrentDistance) / (total_dist - decel_start));
        else
            speed = 0;
    }

    target_speed = (int16_t)speed;
    target_line  = 0;
    PID_Calc();

    /* 走够 1.5 米 → 停车 */
    if (CurrentDistance >= TASK3_TARGET_MM)
    {
        Set_PWM(0, 0);
        task_mode = TASK_IDLE;
        Task_Timer_Stop();
        Stopwatch_Stop();
    }
}
/* ================================================================ */
/*      任务四/五：循迹跑一圈经过A点（≤30秒，不要求精确停车）        */
/* ================================================================ */

/* 复用任务一的状态机，去掉 STOP_DELAY */
#define LAP45_RUN_SPEED  600      // 任务四五循迹速度（比任务一慢）
static float lr4_yaw_total = 0;
static float lr4_yaw_last = 0;
static LapRunState_t lr4_state = LR_FOLLOW;

void Task4_start(void)
{
    Pendulum_SendCmd(0x03);  // 通知摆杆：任务5（球稳定在O，跑一圈）
    task_mode = TASK_4;
    Task_Init();
    pid_speed.Enabled = 1;
    pid_line.Enabled  = 1;
    pid_steer.Enabled = 0;
    lr4_state = LR_FOLLOW;
    lr4_yaw_total = 0;
    lr4_yaw_last = yaw_angle;
}

void Task4(void)
{
    CurrentDistance = ((float)distA_mm + (float)distB_mm) / 2.0f;

    /* 更新累计航向角 */
    float delta = yaw_angle - lr4_yaw_last;
    if (delta > 180.0f)  delta -= 360.0f;
    if (delta < -180.0f) delta += 360.0f;
    lr4_yaw_total += delta;
    lr4_yaw_last = yaw_angle;

    /* 梯形速度曲线 */
    float speed = 0;
    float base_speed  = 200.0f;    // 起步基础速度
    float accel_dist  = 500.0f;    // 加速段 500mm
    float decel_start = 5800.0f;   // 5.8m 开始减速
    float stop_dist   = 6200.0f;   // 6.2m 停车

    if (CurrentDistance < accel_dist)
        speed = base_speed + (LAP45_RUN_SPEED - base_speed) * (CurrentDistance / accel_dist);
    else if (CurrentDistance < decel_start && lr4_yaw_total < 360.0f)
        speed = LAP45_RUN_SPEED;
    else
    {
        /* 进入减速段 → 停计时 */
        Stopwatch_Stop();
        if (CurrentDistance < stop_dist)
            speed = LAP45_RUN_SPEED * ((stop_dist - CurrentDistance) / (stop_dist - decel_start));
        else
            speed = 0;
    }

    target_speed = (int16_t)speed;
    target_line  = 0;
    PID_Calc();

    /* 距离到 或 角度到 → 停车 */
    if (CurrentDistance >= stop_dist || lr4_yaw_total >= 360.0f)
    {
        Set_PWM(0, 0);
        task_mode = TASK_IDLE;
        Task_Timer_Stop();
        Stopwatch_Stop();
    }
}

/* 任务五：跟任务四完全一样 */
void Task5_start(void)
{
    Pendulum_SendCmd(0x04);  // 通知摆杆：任务6（球在任意位置，跑一圈）
    task_mode = TASK_5;
    Task_Init();
    pid_speed.Enabled = 1;
    pid_line.Enabled  = 1;
    pid_steer.Enabled = 0;
    lr4_state = LR_FOLLOW;
    lr4_yaw_total = 0;
    lr4_yaw_last = yaw_angle;
}
void Task5(void)
{
    Task4();
}