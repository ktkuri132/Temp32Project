#ifndef __SH1106_H
#define __SH1106_H

#include <stdint.h>
#include <stdbool.h>

/* ================= 配置区域 ================= */

// 选择驱动模式：只能选择一个为 1
#define SH1106_MODE_I2C 1 // 1: 使用 I2C 模式
#define SH1106_MODE_SPI 0 // 1: 使用 SPI 模式 (4-wire)

/* ================= 硬件接口定义 ================= */

#if SH1106_MODE_I2C

// 定义 I2C 地址 (7-bit address)
#define SH1106_I2C_ADDR 0x3C

// I2C 接口结构体
typedef struct
{
    // 发送函数：addr(7bit设备地址), data(数据指针), len(数据长度)
    // 返回值：0成功，其他失败
    int (*Transmit)(uint8_t addr, uint8_t *data, uint16_t len);
    // 延时函数 (可选，用于复位等)
    void (*DelayMs)(uint32_t ms);
} SH1106_IO_t;

#elif SH1106_MODE_SPI

// SPI 接口结构体
typedef struct
{
    // 片选控制: 0=Low(Select), 1=High(Deselect)
    void (*CS_Control)(uint8_t state);
    // 数据/命令选择: 0=Command, 1=Data
    void (*DC_Control)(uint8_t state);
    // 复位控制: 0=Reset, 1=Work
    void (*RES_Control)(uint8_t state);
    // SPI发送数据: data(数据指针), len(数据长度)
    int (*Transmit)(uint8_t *data, uint16_t len);
    // 延时函数
    void (*DelayMs)(uint32_t ms);
} SH1106_IO_t;

#endif

/* ================= 显存参数 ================= */
#define SH1106_WIDTH 128
#define SH1106_HEIGHT 64
#define SH1106_PAGES 8

/* ================= 函数声明 ================= */

// 注册底层IO接口
void SH1106_Register_IO(SH1106_IO_t *io_ptr);

// 初始化
void SH1106_Init(void);

// 全屏清除
void SH1106_Clear_All(void);

// 部分清除 (清除指定页的指定列范围)
// page: 0-7, start_col: 0-127, end_col: 0-127
void SH1106_Clear_Part(uint8_t page, uint8_t start_col, uint8_t end_col);
// 将显存缓冲区写入屏幕
void SH1106_Update(void);

// 直接写入显存的一个字节 (用于高级绘图库对接)
void SH1106_SetPixel(uint8_t x, uint8_t y, uint8_t color);

#endif // __SH1106_H
