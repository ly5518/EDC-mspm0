#ifndef _TASK_H
#define _TASK_H

#include "motor.h"
#include "encoder.h"
#include "pid.h"

/* ============ PWM限幅 ============ */
#define PWM_MAX  7000
#define PWM_MIN -7000

/* ============ 任务模式 ============ */
typedef enum {
    TASK_IDLE = 0,
    TASK_SQUARE,
    TASK_LINE_SQUARE,
} TaskMode_t;

/* ============ 任务节点 ============ */
typedef enum {
    POINT_A = 0,
    POINT_B,
    POINT_C,
    POINT_D,
} TaskPoint_t;

/* ============ PID 实例 ============ */
extern PID_t pid_speed;
extern PID_t pid_steer;
extern PID_t pid_line;

/* ============ 目标值 ============ */
extern int16_t target_speed;
extern float   target_yaw;
extern int16_t target_line;

/* ============ 循迹状态 ============ */
extern int16_t line_pos;

/* ============ 任务状态 ============ */
extern TaskMode_t task_mode;
extern TaskPoint_t current_point;
extern float CurrentDistance;

/* ============ 正方形参数 ============ */
extern float sq_target;
extern float sq_turn_target;
extern int16_t sq_target_speed;

/* ============ 函数 ============ */
void Task_Init(void);
void Motor_PID_Sync(void);
void Line_CalcPosition(void);
void Task_Run(void);

/* ============ 正方形参数 ============ */
extern uint8_t ls_lap_target;  // 循迹圈数目标

/* ============ 任务函数 ============ */
void Task_Square_Start(void);
void Task_Square(void);
void Task_LineSquare_Start(void);
void Task_LineSquare(void);

#endif
