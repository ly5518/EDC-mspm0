#include "pid.h"
#include <stdlib.h>

static int16_t clamp(int32_t val, int16_t min, int16_t max)
{
    if (val > max) return max;
    if (val < min) return min;
    return (int16_t)val;
}

void PID_Init(PID_TypeDef *pid, float kp, float ki, float kd,
              int16_t deadzone, int16_t max_out, int16_t integral_limit)
{
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;
    pid->deadzone = deadzone;
    pid->max_out = max_out;
    pid->min_out = -max_out;
    pid->integral_limit = integral_limit;

    pid->target = 0;
    pid->measured = 0;
    pid->output = 0;
    pid->integral = 0;
    pid->last_error = 0;
    pid->last_last_error = 0;
}

void PID_Reset(PID_TypeDef *pid)
{
    pid->output = 0;
    pid->integral = 0;
    pid->last_error = 0;
    pid->last_last_error = 0;
}

void PID_SetKp(PID_TypeDef *pid, float kp) { pid->Kp = kp; }
void PID_SetKi(PID_TypeDef *pid, float ki) { pid->Ki = ki; }
void PID_SetKd(PID_TypeDef *pid, float kd) { pid->Kd = kd; }

/**
 * @brief 增量式PID
 *        Δout = Kp*(e-e1) + Ki*e + Kd*(e-2*e1+e2)
 *        out += Δout
 */
int16_t PID_Incremental_Calc(PID_TypeDef *pid)
{
    int16_t error = pid->target - pid->measured;

    // 死区
    if (abs(error) < pid->deadzone)
        error = 0;

    // 增量计算（用 int32_t 防溢出）
    int32_t delta = (int32_t)(pid->Kp * (error - pid->last_error))
                  + (int32_t)(pid->Ki * error)
                  + (int32_t)(pid->Kd * (error - 2 * pid->last_error + pid->last_last_error));

    int32_t new_out = pid->output + delta;
    pid->output = clamp(new_out, pid->min_out, pid->max_out);

    pid->last_last_error = pid->last_error;
    pid->last_error = error;

    return pid->output;
}

/**
 * @brief 位置式PID
 *        out = Kp*e + Ki*Σe + Kd*(e-e1)
 */
int16_t PID_Positional_Calc(PID_TypeDef *pid)
{
    int16_t error = pid->target - pid->measured;

    // 死区
    if (abs(error) < pid->deadzone)
        error = 0;

    // 积分累加 + 限幅
    pid->integral += error;
    if (pid->integral > pid->integral_limit)
        pid->integral = pid->integral_limit;
    else if (pid->integral < -pid->integral_limit)
        pid->integral = -pid->integral_limit;

    // 位置式输出
    int32_t out = (int32_t)(pid->Kp * error)
                + (int32_t)(pid->Ki * pid->integral)
                + (int32_t)(pid->Kd * (error - pid->last_error));

    pid->output = clamp(out, pid->min_out, pid->max_out);
    pid->last_error = error;

    return pid->output;
}
