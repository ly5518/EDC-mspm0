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
    .Kp = 15.8f, .Ki = 0.1f, .Kd = 0.0f,
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
    .Kp = 40.0f, .Ki = 0.0f, .Kd = 0.1f,
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
static const int8_t line_weight[4] = {-24, -10, 10, 24};

/* ============ 任务状态 ============ */
TaskMode_t task_mode = TASK_IDLE;
float CurrentDistance = 0.0f;

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
#define LAP_SLOW_DOWN_DIST   5600.0f  // 距离到了 5.5 米 → 减速
#define LAP_SLOW_DOWN_YAW    320.0f   // 累计角度 > 320° → 确认快走完一圈
#define LAP_STOP_DIST        5900.0f  // 停车距离（一圈约6.14米）
#define LAP_STOP_YAW         360.0f   // 停车角度

typedef enum {
    LR_FOLLOW,      // 循迹行驶
    LR_SLOW_DOWN,   // 减速循迹
    LR_STOP         // 停车
} LapRunState_t;

static LapRunState_t lr_state = LR_FOLLOW;
static float yaw_start = 0;           // 启动时的yaw角度
static int16_t t1_last_speed = 0;     // 速度斜坡状态

void Task1_start(void)
{
    Task_Timer_Start();
    task_mode = TASK_1;
    Task_Init();
    t1_last_speed = 0;
    pid_speed.Enabled = 1;
    pid_line.Enabled  = 1;
    pid_steer.Enabled = 0;
    lr_state = LR_FOLLOW;
    yaw_start = yaw_angle;  // 记录启动时的朝向
}

void Task1(void)
{
    CurrentDistance = ((float)distA_mm + (float)distB_mm) / 2.0f;

    /* 计算yaw偏差：当前朝向 vs 启动朝向 */
    float yaw_err = yaw_angle - yaw_start;
    if (yaw_err > 180.0f)  yaw_err -= 360.0f;
    if (yaw_err < -180.0f) yaw_err += 360.0f;

    /* 每500ms打印 */
    static uint32_t t1_dbg = 0;
    if (++t1_dbg >= 50) { t1_dbg = 0; printf("T1: dist=%.0f yaw_err=%.1f state=%d\r\n", CurrentDistance, yaw_err, lr_state); }

    float speed = 0;

    switch (lr_state)
    {
    /* ---- 正常循迹 ---- */
    case LR_FOLLOW:
    {
        speed = LAP_RUN_SPEED;

        if (CurrentDistance >= LAP_SLOW_DOWN_DIST)
        {
            lr_state = LR_SLOW_DOWN;
            printf("T1: SLOW_DOWN dist=%.0f yaw_err=%.1f\r\n", CurrentDistance, yaw_err);
        }
        break;
    }

    /* ---- S曲线减速 ---- */
    case LR_SLOW_DOWN:
    {
        float progress = (CurrentDistance - LAP_SLOW_DOWN_DIST) / (LAP_STOP_DIST - LAP_SLOW_DOWN_DIST);
        if (progress > 1.0f) progress = 1.0f;
        if (progress < 0.0f) progress = 0.0f;
        float smooth = progress * progress * (3.0f - 2.0f * progress);
        speed = 200.0f + (LAP_RUN_SPEED - 200.0f) * (1.0f - smooth);

        /* 计算yaw偏差：当前朝向 vs 启动朝向 */
        float yaw_err = yaw_angle - yaw_start;
        if (yaw_err > 180.0f)  yaw_err -= 360.0f;
        if (yaw_err < -180.0f) yaw_err += 360.0f;

        /* 停车条件：距离到了 + 方向回正，或 兜底距离 */
        uint8_t dist_ok = (CurrentDistance >= LAP_STOP_DIST * 0.92f);
        uint8_t angle_ok = (fabs(yaw_err) < 8.0f);
        uint8_t fallback = (CurrentDistance >= 5950.0f);

        if ((dist_ok && angle_ok) || fallback)
        {
            printf("T1 STOP: dist=%.0f yaw_err=%.1f fb=%d\r\n",
                   CurrentDistance, yaw_err, fallback);
            lr_state = LR_STOP;
        }
        break;
    }

    /* ---- 停车：保持TASK_1模式继续打印 ---- */
    case LR_STOP:
    {
        Set_PWM(0, 0);
        Task_Timer_Stop();
        Stopwatch_Stop();
        return;  // 直接返回，跳过后面的PID计算
    }
    default:
        break;
    }

    /* 速度斜坡：限制每步变化量 */
    #define T1_SPEED_STEP 10
    int16_t target = (int16_t)speed;
    if (target > t1_last_speed + T1_SPEED_STEP) target = t1_last_speed + T1_SPEED_STEP;
    if (target < t1_last_speed - T1_SPEED_STEP) target = t1_last_speed - T1_SPEED_STEP;
    t1_last_speed = target;

    target_speed = target;
    target_line  = 0;
    PID_Calc();
}
void Task2_start(void)
{
    Pendulum_SendCmd(0x01);  // 通知摆杆：任务3（球从O到±5cm）
    task_mode = TASK_2;
    Task_Init();
}
void Task2(void)
{
    Set_PWM(0, 0);
}
/* ================================================================ */
/*           任务三：AB段循迹（1.5米，≤8秒）                         */
/* ================================================================ */

#define TASK3_SPEED     520      // 巡迹速度
#define TASK3_TARGET_MM 1700.0f  // 总行驶距离（1.5米处停计时，之后缓停）

static int16_t t3_last_speed = 0;  // 任务三速度斜坡状态

void Task3_start(void)
{
    Task_Timer_Start();
    Pendulum_SendCmd(0x02);  // 通知摆杆：任务4（球稳定在O，A→B）
    task_mode = TASK_3;
    Task_Init();
    pid_speed.Enabled = 1;
    pid_line.Enabled  = 1;
    pid_steer.Enabled = 0;
    t3_last_speed = 0;  // 重置速度斜坡
}

void Task3(void)
{
    CurrentDistance = ((float)distA_mm + (float)distB_mm) / 2.0f;

    /* 速度曲线：加速 → 匀速 → 缓减速 → 滑停 */
    float speed = 0;
    float base_speed  = 180.0f;    // 起步基础速度
    float accel_end   = 400.0f;    // 加速段 400mm
    float cruise_end  = 1200.0f;   // 匀速段结束
    float timer_stop  = 1500.0f;   // 1500mm 停计时（通过B点）
    float total_dist  = TASK3_TARGET_MM;  // 1700mm 总距离

    if (CurrentDistance < accel_end)
    {
        /* S曲线加速：base_speed → TASK3_SPEED */
        float x = CurrentDistance / accel_end;
        speed = base_speed + (TASK3_SPEED - base_speed) * (3*x*x - 2*x*x*x);
    }
    else if (CurrentDistance < cruise_end)
    {
        /* 匀速段 */
        speed = TASK3_SPEED;
    }
    else if (CurrentDistance < timer_stop)
    {
        /* S曲线收速：TASK3_SPEED → 450（过B点前收速，保护小球） */
        float t = (CurrentDistance - cruise_end) / (timer_stop - cruise_end);
        speed = TASK3_SPEED - 70.0f * (3*t*t - 2*t*t*t);
    }
    else if (CurrentDistance < total_dist)
    {
        /* S曲线缓减：450 → 80 */
        Stopwatch_Stop();
        float t = (CurrentDistance - timer_stop) / (total_dist - timer_stop);
        speed = 450.0f - 370.0f * (3*t*t - 2*t*t*t);
    }
    else
    {
        /* 到达1800mm，PID自然减速到0 */
        Stopwatch_Stop();
        speed = 0;
        /* 实际速度降到30以下 或 超时2秒 → 真正停车 */
        static uint32_t stop_tick = 0;
        if (stop_tick == 0) stop_tick = menu_count;
        if (((float)speedA + (float)speedB) / 2.0f < 30.0f || (menu_count - stop_tick) > 2000)
        {
            Set_PWM(0, 0);
            task_mode = TASK_IDLE;
            stop_tick = 0;
            return;
        }
    }

    /* PWM斜坡：限制速度变化率，每步最多变化5 */
    #define SPEED_STEP 5
    int16_t target = (int16_t)speed;
    if (target > t3_last_speed + SPEED_STEP) target = t3_last_speed + SPEED_STEP;
    if (target < t3_last_speed - SPEED_STEP) target = t3_last_speed - SPEED_STEP;
    t3_last_speed = target;

    target_speed = target;
    target_line  = 0;
    PID_Calc();

    /* 每200ms打印速度和距离 */
    static uint32_t t3_dbg = 0;
    if (++t3_dbg >= 20) { t3_dbg = 0; printf("T3: dist=%.0f spd=%d actual=%d\r\n", CurrentDistance, target, (speedA+speedB)/2); }
}
/* ================================================================ */
/*      任务四/五：循迹跑一圈经过A点                                  */
/* ================================================================ */

#define LAP45_RUN_SPEED  520
static float lr4_yaw_total = 0;
static float lr4_yaw_last = 0;
static LapRunState_t lr4_state = LR_FOLLOW;
static int16_t t4_last_speed = 0;  // 任务四速度斜坡状态

void Task4_start(void)
{
    Pendulum_SendCmd(0x03);
    task_mode = TASK_4;
    Task_Init();
    pid_speed.Enabled = 1;
    pid_line.Enabled  = 1;
    pid_steer.Enabled = 0;
    lr4_state = LR_FOLLOW;
    lr4_yaw_total = 0;
    lr4_yaw_last = yaw_angle;
    t4_last_speed = 0;
    Task_Timer_Start();
}

void Task4(void)
{
    CurrentDistance = ((float)distA_mm + (float)distB_mm) / 2.0f;

    float delta = yaw_angle - lr4_yaw_last;
    if (delta > 180.0f)  delta -= 360.0f;
    if (delta < -180.0f) delta += 360.0f;
    lr4_yaw_total += delta;
    lr4_yaw_last = yaw_angle;

    /* 弯道降速：line_pos越大（偏离中心越多）→ 速度越低（低通滤波防抖） */
    static float curve_factor_smooth = 1.0f;
    float curve_factor_raw = 1.0f - 0.5f * ((float)abs(line_pos) / 20.0f);
    if (curve_factor_raw < 0.5f) curve_factor_raw = 0.5f;
    curve_factor_smooth = curve_factor_smooth * 0.8f + curve_factor_raw * 0.2f;
    float curve_factor = curve_factor_smooth;

    float speed = 0;
    float base_speed  = 200.0f;     // 起步速度
    float max_speed   = LAP45_RUN_SPEED * curve_factor;  // 弯道自动降速
    float accel_dist  = 800.0f;     // 加速段 800mm
    float decel_start = 5500.0f;    // 开始减速
    float a_point     = 5900.0f;    // A点：停计时
    float stop_yaw    = 350.0f;     // A点角度：停计时
    float car_stop    = 6000.0f;    // 停车距离

    /* A点停计时 */
    if (CurrentDistance >= a_point || lr4_yaw_total >= stop_yaw)
        Stopwatch_Stop();

    if (CurrentDistance < accel_dist)
    {
        /* S曲线加速 */
        float x = CurrentDistance / accel_dist;
        float s = x * x * (3.0f - 2.0f * x);
        speed = base_speed + (max_speed - base_speed) * s;
    }
    else if (CurrentDistance < decel_start && lr4_yaw_total < 360.0f)
    {
        speed = max_speed;
    }
    else
    {
        if (CurrentDistance < car_stop)
        {
            /* S曲线减速，但不低于150 */
            float x = (CurrentDistance - decel_start) / (car_stop - decel_start);
            if (x > 1.0f) x = 1.0f;
            float s = x * x * (3.0f - 2.0f * x);
            speed = max_speed * (1.0f - s);
            if (speed < 150.0f) speed = 150.0f;
        }
        else
        {
            Set_PWM(0, 0);
            task_mode = TASK_IDLE;
            Task_Timer_Stop();
            return;
        }
    }

    /* 速度斜坡：限制每步变化量，防止突然加减速 */
    #define T4_SPEED_STEP 3  // 每10ms最多变化3（更平缓）
    int16_t target = (int16_t)speed;
    if (target > t4_last_speed + T4_SPEED_STEP) target = t4_last_speed + T4_SPEED_STEP;
    if (target < t4_last_speed - T4_SPEED_STEP) target = t4_last_speed - T4_SPEED_STEP;
    t4_last_speed = target;

    target_speed = target;
    target_line  = 0;
    PID_Calc();
}

/* 任务五：跟任务四完全一样 */
void Task5_start(void)
{
    Pendulum_SendCmd(0x04);
    task_mode = TASK_5;
    Task_Init();
    pid_speed.Enabled = 1;
    pid_line.Enabled  = 1;
    pid_steer.Enabled = 0;
    lr4_state = LR_FOLLOW;
    lr4_yaw_total = 0;
    lr4_yaw_last = yaw_angle;
    t4_last_speed = 0;
    Task_Timer_Start();
}
void Task5(void)
{
    Task4();
}