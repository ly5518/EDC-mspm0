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
    TASK_1,
    TASK_2,
    TASK_3,
    TASK_4,
    TASK_5,
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
extern float CurrentDistance;

/* ============ 函数 ============ */
void Task_Init(void);
void Motor_PID_Sync(void);
void Line_CalcPosition(void);
void Task_Run(void);

/* ============ 菜单触发标志 ============ */
extern volatile uint8_t task1_flag;
extern volatile uint8_t task2_flag;
extern volatile uint8_t task3_flag;
extern volatile uint8_t task4_flag;
extern volatile uint8_t task5_flag;

/* ============ 任务一函数 ============ */
void Task1_start(void);
void Task1(void);
void Task2_start(void);
void Task2(void);
void Task3_start(void);
void Task3(void);
void Task4_start(void);
void Task4(void);
void Task5_start(void);
void Task5(void);

/* ============ 任务计时器 ============ */
extern volatile uint32_t task_elapsed_ms;    // 任务已运行毫秒数
extern volatile uint8_t  task_timer_running; // 计时器是否在跑
extern uint32_t menu_count;                  // 来自 empty.c，1ms 递增

void Task_Timer_Start(void);
void Task_Timer_Stop(void);

#endif
