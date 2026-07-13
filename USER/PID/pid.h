#ifndef _PID_H
#define _PID_H

#include <stdint.h>

typedef struct {
    /* 参数 */
    float Kp;
    float Ki;
    float Kd;
    int16_t deadzone;       // 死区（误差小于此值不调节）
    int16_t max_out;        // 输出上限
    int16_t min_out;        // 输出下限（通常 = -max_out）
    int16_t integral_limit; // 积分限幅

    /* 运行时状态 */
    int16_t target;
    int16_t measured;
    int16_t output;
    int32_t integral;
    int16_t last_error;
    int16_t last_last_error;
} PID_TypeDef;

void PID_Init(PID_TypeDef *pid, float kp, float ki, float kd,
              int16_t deadzone, int16_t max_out, int16_t integral_limit);
void PID_Reset(PID_TypeDef *pid);
void PID_SetKp(PID_TypeDef *pid, float kp);
void PID_SetKi(PID_TypeDef *pid, float ki);
void PID_SetKd(PID_TypeDef *pid, float kd);
int16_t PID_Incremental_Calc(PID_TypeDef *pid);
int16_t PID_Positional_Calc(PID_TypeDef *pid);

#endif
