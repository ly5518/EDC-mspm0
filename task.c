/**
 * @file    task.c
 * @brief   任务架构：PID + 正方形任务
 */

#include "task.h"
#include <math.h>
#include <stdio.h>

extern float yaw_angle;

/* ============ PID 实例 ============ */
PID_t pid_speed = {
    .Kp = 1.8f, .Ki = 0.1f, .Kd = 0.0f,
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
    .Kp = 45.0f, .Ki = 0.0f, .Kd = 0.1f,
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
static const int8_t line_weight[8] = {-60, -40, -20, -8, 8, 20, 40, 60};

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

/* ============ 循迹正方形参数 ============ */
uint8_t ls_lap_target = 1;       // 目标圈数
static uint8_t ls_lap_count = 0; // 已走圈数
static float ls_yaw_sum = 0;     // IMU累计角度
static float ls_last_yaw = 0;    // 上一次yaw

/* ================================================================ */
void Task_Init(void)
{
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
    extern char gray_digital_str[9];
    int32_t sum = 0;
    uint8_t count = 0;
    for (int i = 0; i < 8; i++)
    {
        if (gray_digital_str[i] == '1')
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
    int LeftPWM  = -(pwm - steer + line);
    int RightPWM = -(pwm + steer - line);
    LeftPWM  = (LeftPWM  > PWM_MAX) ? PWM_MAX : (LeftPWM  < PWM_MIN ? PWM_MIN : LeftPWM);
    RightPWM = (RightPWM > PWM_MAX) ? PWM_MAX : (RightPWM < PWM_MIN ? PWM_MIN : RightPWM);
    Set_PWM(LeftPWM, RightPWM);
}

/* ============ 任务调度 ============ */
void Task_Run(void)
{
    switch (task_mode)
    {
    case TASK_IDLE:
        Set_PWM(0, 0);
        break;
    case TASK_SQUARE:
        Task_Square();
        break;
    case TASK_LINE_SQUARE:
        Task_LineSquare();
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
    extern char gray_digital_str[9];

    if (lf_state == LF_FOLLOW)
    {
        pid_speed.Enabled = 1;
        pid_line.Enabled = 1;
        pid_steer.Enabled = 0;
        target_speed = sq_target_speed;
        target_line = 0;
        PID_Calc();

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

        // 转够80° 且 线重新出现 → 回到循迹
        float turned = yaw_angle - lf_turn_start;
        if (turned > 180.0f)  turned -= 360.0f;
        if (turned < -180.0f) turned += 360.0f;

        uint8_t line_found = 0;
        for (int i = 0; i < 8; i++)
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

