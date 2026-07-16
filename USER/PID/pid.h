/**
 ******************************************************************************
 * @file    PID.h
 * @brief   PID控制器头文件 - 支持位置式/增量式/微分先行三种模式
 ******************************************************************************
 */

#ifndef __PID_H
#define __PID_H

#include <stdint.h>
#include <math.h>

/**
 * @brief PID控制模式枚举定义
 */
typedef enum {
    PID_POSITION = 0,                 /*!< 位置式PID - 输出绝对值 */
    PID_INCREMENTAL = 1,              /*!< 增量式PID - 输出相对变化量 */
    PID_DERIVATIVE_ON_MEASUREMENT = 2 /*!< 微分先行PID - 微分作用于测量值，抗干扰性强（推荐） */
} PID_Mode_t;

/**
 * @brief PID控制器结构体定义
 */
typedef struct {
    /* 核心PID参数 */
    float Kp;
    float Ki;
    float Kd;

    /* 控制目标与反馈 */
    float Target;
    float Actual;

    /* 误差计算相关 */
    float Error[3];       // [0]:当前误差 [1]:上次误差 [2]:上上次误差
    float ErrorInt;       // 积分累积值
    float ErrorDer;       // 微分值

    /* 输出限制 */
    float OutMax;
    float OutMin;

    /* 积分限制 */
    float IntegralMax;
    float IntegralMin;

    /* 死区控制 */
    float DeadZone;
    uint8_t DeadZoneEn;

    /* 积分分离 */
    float IntegralSepThres;
    uint8_t IntegralSepEn;

    /* PID使能 */
    uint8_t Enabled;

    /* 输出结果 */
    float Out;

    /* 工作模式 */
    PID_Mode_t Mode;
} PID_t;

/* 函数声明 */
void PID_Init(PID_t *pid);
void PID_SetParam(PID_t *pid, float Kp, float Ki, float Kd);
void PID_SetLimits(PID_t *pid, float OutMax, float OutMin, float IntegralMax, float IntegralMin);
void PID_SetDeadZone(PID_t *pid, float DeadZone, uint8_t Enable);
void PID_SetIntegralSep(PID_t *pid, float Threshold, uint8_t Enable);
void PID_SetMode(PID_t *pid, PID_Mode_t Mode);
void PID_SetTarget(PID_t *pid, float Target);
void PID_Update(PID_t *pid, float Actual);
void PID_UpdateAngle(PID_t *pid, float Actual);
void PID_Clear(PID_t *pid);

/* 全局PID参数（菜单可调） */
extern float speed_Kp, speed_Ki, speed_Kd;
extern float angle_Kp, angle_Ki, angle_Kd;
extern float line_Kp, line_Ki, line_Kd;

#endif
