/*
 * 立创开发板硬件例程
 * 立创开发板：www.lckfb.com
 * 技术支持：wiki.lckfb.com
 * 技术支持驻守论坛，任何技术问题欢迎随时学习交流
 * 技术支持社区：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 我们不生产代码，我们只是代码的搬运工
 */

#ifndef __BSP_SG90_H__
#define __BSP_SG90_H__

#include "board.h"

void Set_Servo_Angle(unsigned int angle);
unsigned int Get_Servo_Angle(void);

#endif /* __BSP_SG90_H__ */
