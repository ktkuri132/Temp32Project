#ifndef __TERMINAL_LINK_H
#define __TERMINAL_LINK_H

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>


typedef struct {
    char c;               // 单次数据
    int64_t data;         // 数据
    uint8_t color;        // 字体颜色
    uint8_t sit_x;        // 字体坐标x
    uint8_t sit_y;        // 字体坐标y
    uint8_t fresh;        // 刷新标志位,标志字符是否需要刷新
    uint8_t color_fresh;  // 刷新颜色标志位,标志字符颜色是否需要刷新
    uint8_t sit_fresh;    // 刷新坐标标志位,标志字符坐标是否需要刷新
} GraphicsChar_Unit;      // 字符结构体

void Wirte_Char(uint8_t x, uint8_t y, char c, uint8_t color);
void Read_Char(uint8_t x, uint8_t y, char *c, uint8_t *color);
void Clear_Char(uint8_t x, uint8_t y);
void Clear_Screen();
void Graphics_UpMove();
void Wirte_String(uint8_t x, uint8_t y, uint8_t color, char *str, ...);
void Read_String(uint8_t x, uint8_t y, char *str, uint8_t color);
void refresh_Allscreen();
void refresh_Partscreen(uint8_t x, uint8_t y, uint8_t Mode);

void loading_bar(uint8_t x, uint8_t y, uint8_t color, int sta, int end,
                 int curr);

#endif