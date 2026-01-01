#ifndef __ST7789_H
#define __ST7789_H

#ifdef USE_DEVICE_ST7789

#include <stdint.h>
#include <stdbool.h>

/* ================= 配置区域 ================= */

// ST7789 通常使用 SPI 接口
#define ST7789_MODE_SPI 1

/* ================= 硬件接口定义 ================= */

#if ST7789_MODE_SPI

// SPI 接口结构体
typedef struct
{
    // 片选控制: 0=Low(Select), 1=High(Deselect)
    void (*CS_Control)(uint8_t state);
    // 数据/命令选择: 0=Command, 1=Data
    void (*DC_Control)(uint8_t state);
    // 复位控制: 0=Reset, 1=Work
    void (*RES_Control)(uint8_t state);
    // 背光控制 (可选)
    void (*BLK_Control)(uint8_t state);
    // SPI发送数据: data(数据指针), len(数据长度)
    int (*Transmit)(uint8_t *data, uint16_t len);
    // 延时函数
    void (*DelayMs)(uint32_t ms);
} ST7789_IO_t;

#endif

/* ================= 显存参数 ================= */
#define ST7789_WIDTH 240
#define ST7789_HEIGHT 240

// 常用颜色定义 (RGB565)
#define ST7789_BLACK 0x0000
#define ST7789_BLUE 0x001F
#define ST7789_RED 0xF800
#define ST7789_GREEN 0x07E0
#define ST7789_CYAN 0x07FF
#define ST7789_MAGENTA 0xF81F
#define ST7789_YELLOW 0xFFE0
#define ST7789_WHITE 0xFFFF

/* ================= 函数声明 ================= */

// 注册底层IO接口
void ST7789_Register_IO(ST7789_IO_t *io_ptr);

// 初始化
void ST7789_Init(void);

// 全屏填充颜色
void ST7789_Clear_All(uint16_t color);

// 区域填充颜色
void ST7789_Fill_Rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

// 画点
void ST7789_SetPixel(uint16_t x, uint16_t y, uint16_t color);

// 写入图片数据 (RGB565)
void ST7789_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data);

#endif // USE_DEVICE_ST7789

#endif // __ST7789_H
