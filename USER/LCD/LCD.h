#ifndef __LCD_H_
#define __LCD_H_

#include "ti_msp_dl_config.h"
#include "LCD_Font.h"
#include <stdarg.h>
#include "stdlib.h"
#include <string.h>
#include <stdio.h>
#include "board.h"

//---------------------------------------------------------------------------------------------------------------
// #define SPI_INST     (SPI_INST)           // SPI通道
#define GPIO_PORT    (LCD_PORT) // GPIO_PORT
#define GPIO_RES_PIN (LCD_RES_PIN)   // RES
#define GPIO_DC_PIN  (LCD_DC_PIN)    // DC
#define GPIO_CS_PIN  (LCD_CS_PIN)    // CS
//---------------------------------------------------------------------------------------------------------------
// LCD驱动选择: 0 = ST7735S (1.8寸 128x160), 1 = ILI9341 (2.4寸 240x320)
#define LCD_DRIVER               (0)

#define LCD_DEFAULT_DISPLAY_DIR  (LCD_DRIVER == 0) ? LCD_PORTAIT : LCD_PORTAIT_180  // 默认的显示方向
#define LCD_DEFAULT_PENCOLOR     (RGB565_WHITE) // 默认的画笔颜色
#define LCD_DEFAULT_BGCOLOR      (RGB565_BLACK) // 默认的背景颜色
#define LCD_DEFAULT_DISPLAY_FONT (LCD_6X8_FONT) // 默认的字体模式

#if (LCD_DRIVER == 0)
#define LCD_W                    128            // ST7735S 屏幕尺寸W
#define LCD_H                    160            // ST7735S 屏幕尺寸H
#elif (LCD_DRIVER == 1)
#define LCD_W                    240            // ILI9341 屏幕尺寸W
#define LCD_H                    320            // ILI9341 屏幕尺寸H
#endif

#define LCD_RES(x)               ((x) ? DL_GPIO_setPins(GPIO_PORT, GPIO_RES_PIN) : DL_GPIO_clearPins(GPIO_PORT, GPIO_RES_PIN))
#define LCD_DC(x)                ((x) ? DL_GPIO_setPins(GPIO_PORT, GPIO_DC_PIN) : DL_GPIO_clearPins(GPIO_PORT, GPIO_DC_PIN))
#define LCD_CS(x)                ((x) ? DL_GPIO_setPins(GPIO_PORT, GPIO_CS_PIN) : DL_GPIO_clearPins(GPIO_PORT, GPIO_CS_PIN))

//---------------------------------------------------------------------------------------------------------------
typedef enum {
    LCD_PORTAIT       = 0, // 竖屏模式
    LCD_PORTAIT_180   = 1, // 竖屏模式  旋转180
    LCD_CROSSWISE     = 2, // 横屏模式
    LCD_CROSSWISE_180 = 3, // 横屏模式  旋转180
} LCD_dir_enum;

typedef enum {
    LCD_6X8_FONT   = 0, // 6x8      字体
    LCD_8X16_FONT  = 1, // 8x16     字体
    LCD_16X16_FONT = 2, // 16x16    字体 目前不支持
} LCD_font_size_enum;
//---------------------------------------------------------------------------------------------------------------

void LCD_init(void);

void LCD_clear(void);
void LCD_full(const uint16_t color);
void LCD_set_color(uint16_t pen, const uint16_t bgcolor);
void LCD_set_display_font(LCD_font_size_enum font);
void LCD_draw_point(uint16_t x, uint16_t y, const uint16_t color);
void LCD_draw_line(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, const uint16_t color);
void LCD_show_char(uint16_t x, uint16_t y, const char dat);
void LCD_show_string(uint16_t x, uint16_t y, const char dat[]);
void LCD_show_int(uint16_t x, uint16_t y, const int32_t dat, uint8_t num);
void LCD_show_uint(uint16_t x, uint16_t y, const uint32_t dat, uint8_t num);
void LCD_show_float(uint16_t x, uint16_t y, const double dat, uint8_t num, uint8_t pointnum);
void LCD_fill(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, const uint16_t color);
void LCDprint(uint16_t x, uint16_t y, const char *format, ...);

#endif