/**
 ******************************************************************************
 * @file    PID.c
 * @brief   PID控制器实现 - 位置式/增量式/微分先行
 ******************************************************************************
 */

#include "pid.h"
#include <stddef.h>

/* 全局PID参数（菜单可调） */
float speed_Kp = 1.8f;
float speed_Ki = 0.1f;
float speed_Kd = 0.0f;

float angle_Kp = 80.0f;
float angle_Ki = 1.0f;
float angle_Kd = 3.0f;

float line_Kp = 0.5f;
float line_Ki = 0.0f;
float line_Kd = 0.1f;

void PID_Init(PID_t *pid)
{
    if (pid == NULL) return;
    pid->Kp = 0.0f;
    pid->Ki = 0.0f;
    pid->Kd = 0.0f;
    pid->Target = 0.0f;
    pid->Actual = 0.0f;
    pid->Error[0] = 0.0f;
    pid->Error[1] = 0.0f;
    pid->Error[2] = 0.0f;
    pid->ErrorInt = 0.0f;
    pid->ErrorDer = 0.0f;
    pid->OutMax = 100.0f;
    pid->OutMin = -100.0f;
    pid->IntegralMax = 50.0f;
    pid->IntegralMin = -50.0f;
    pid->DeadZone = 0.0f;
    pid->DeadZoneEn = 0;
    pid->IntegralSepThres = 10.0f;
    pid->IntegralSepEn = 1;
    pid->Enabled = 1;
    pid->Out = 0.0f;
    pid->Mode = PID_DERIVATIVE_ON_MEASUREMENT;
}

void PID_SetParam(PID_t *pid, float Kp, float Ki, float Kd)
{
    if (pid == NULL) return;
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
}

void PID_SetLimits(PID_t *pid, float OutMax, float OutMin, float IntegralMax, float IntegralMin)
{
    if (pid == NULL) return;
    pid->OutMax = OutMax;
    pid->OutMin = OutMin;
    pid->IntegralMax = IntegralMax;
    pid->IntegralMin = IntegralMin;
}

void PID_SetDeadZone(PID_t *pid, float DeadZone, uint8_t Enable)
{
    if (pid == NULL) return;
    pid->DeadZone = DeadZone;
    pid->DeadZoneEn = Enable;
}

void PID_SetIntegralSep(PID_t *pid, float Threshold, uint8_t Enable)
{
    if (pid == NULL) return;
    pid->IntegralSepThres = Threshold;
    pid->IntegralSepEn = Enable;
}

void PID_SetMode(PID_t *pid, PID_Mode_t Mode)
{
    if (pid == NULL) return;
    pid->Mode = Mode;
    PID_Clear(pid);
}

void PID_SetTarget(PID_t *pid, float Target)
{
    if (pid == NULL) return;
    pid->Target = Target;
}

void PID_Clear(PID_t *pid)
{
    if (pid == NULL) return;
    pid->Error[0] = 0.0f;
    pid->Error[1] = 0.0f;
    pid->Error[2] = 0.0f;
    pid->ErrorInt = 0.0f;
    pid->ErrorDer = 0.0f;
    pid->Out = 0.0f;
}

/* 通用PID计算 */
void PID_Update(PID_t *pid, float Actual)
{
    if (pid == NULL) return;

    if (!pid->Enabled) {
        pid->Out = 0.0f;
        PID_Clear(pid);
        return;
    }

    pid->Actual = Actual;
    pid->Error[0] = pid->Target - pid->Actual;

    /* 死区 */
    if (pid->DeadZoneEn && fabs(pid->Error[0]) <= pid->DeadZone) {
        pid->Out = 0.0f;
        pid->ErrorInt = 0.0f;
        pid->Error[2] = pid->Error[1];
        pid->Error[1] = pid->Error[0];
        return;
    }

    switch (pid->Mode) {
    case PID_POSITION:
    {
        if (!(pid->IntegralSepEn && fabs(pid->Error[0]) > pid->IntegralSepThres)) {
            pid->ErrorInt += pid->Error[0] * pid->Ki;
            pid->ErrorInt = fmin(fmax(pid->ErrorInt, pid->IntegralMin), pid->IntegralMax);
        }
        pid->ErrorDer = (pid->Error[0] - pid->Error[1]) * pid->Kd;
        pid->Out = pid->Kp * pid->Error[0] + pid->ErrorInt + pid->ErrorDer;
        break;
    }

    case PID_INCREMENTAL:
    {
        float delta = pid->Kp * (pid->Error[0] - pid->Error[1])
                    + pid->Ki * pid->Error[0]
                    + pid->Kd * (pid->Error[0] - 2*pid->Error[1] + pid->Error[2]);
        pid->Out += delta;
        break;
    }

    case PID_DERIVATIVE_ON_MEASUREMENT:
    {
        float p_term = pid->Kp * pid->Error[0];

        float i_term = 0.0f;
        if (!(pid->IntegralSepEn && fabs(pid->Error[0]) > pid->IntegralSepThres)) {
            pid->ErrorInt += pid->Error[0] * pid->Ki;
            pid->ErrorInt = fmin(fmax(pid->ErrorInt, pid->IntegralMin), pid->IntegralMax);
            i_term = pid->ErrorInt;
        }

        float d_term = -pid->Kd * (pid->Actual - pid->Error[1]);
        pid->Out = p_term + i_term + d_term;
        pid->Error[1] = pid->Actual;
        break;
    }
    }

    pid->Out = fmin(fmax(pid->Out, pid->OutMin), pid->OutMax);
    pid->Error[2] = pid->Error[1];
    pid->Error[1] = pid->Error[0];
}

/* 角度PID（自动处理±180°边界） */
void PID_UpdateAngle(PID_t *pid, float Actual)
{
    if (pid == NULL) return;

    if (!pid->Enabled) {
        pid->Out = 0.0f;
        PID_Clear(pid);
        return;
    }

    pid->Actual = Actual;
    pid->Error[0] = pid->Target - pid->Actual;

    /* 角度归一化 */
    while (pid->Error[0] > 180.0f)  pid->Error[0] -= 360.0f;
    while (pid->Error[0] < -180.0f) pid->Error[0] += 360.0f;

    /* 死区 */
    if (pid->DeadZoneEn && fabs(pid->Error[0]) <= pid->DeadZone) {
        pid->Out = 0.0f;
        pid->ErrorInt = 0.0f;
        pid->Error[2] = pid->Error[1];
        pid->Error[1] = pid->Error[0];
        return;
    }

    switch (pid->Mode) {
    case PID_POSITION:
    {
        if (!(pid->IntegralSepEn && fabs(pid->Error[0]) > pid->IntegralSepThres))
            pid->ErrorInt += pid->Error[0];
        pid->ErrorInt = fmin(fmax(pid->ErrorInt, pid->IntegralMin), pid->IntegralMax);
        pid->ErrorDer = pid->Error[0] - pid->Error[1];
        pid->Out = pid->Kp * pid->Error[0] + pid->Ki * pid->ErrorInt + pid->Kd * pid->ErrorDer;
        pid->Out = fmin(fmax(pid->Out, pid->OutMin), pid->OutMax);
        pid->Error[2] = pid->Error[1];
        pid->Error[1] = pid->Error[0];
        break;
    }

    case PID_INCREMENTAL:
    {
        float DeltaOut = pid->Kp * (pid->Error[0] - pid->Error[1])
                       + pid->Ki * pid->Error[0]
                       + pid->Kd * (pid->Error[0] - 2*pid->Error[1] + pid->Error[2]);
        pid->Out += DeltaOut;
        pid->Out = fmin(fmax(pid->Out, pid->OutMin), pid->OutMax);
        pid->Error[2] = pid->Error[1];
        pid->Error[1] = pid->Error[0];
        break;
    }

    case PID_DERIVATIVE_ON_MEASUREMENT:
    {
        if (!(pid->IntegralSepEn && fabs(pid->Error[0]) > pid->IntegralSepThres))
            pid->ErrorInt += pid->Error[0];
        pid->ErrorInt = fmin(fmax(pid->ErrorInt, pid->IntegralMin), pid->IntegralMax);
        pid->ErrorDer = pid->Actual - pid->Error[1];
        pid->Out = pid->Kp * pid->Error[0] + pid->Ki * pid->ErrorInt - pid->Kd * pid->ErrorDer;
        pid->Out = fmin(fmax(pid->Out, pid->OutMin), pid->OutMax);
        pid->Error[2] = pid->Error[1];
        pid->Error[1] = pid->Actual;
        break;
    }
    }
}
