#ifndef __DF_SPI_H_
#define __DF_SPI_H_

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <dev_frame.h>

#define CPOL 0 // 时钟极性
#define CPHA 0 // 时钟相位

#define MODE1 1 // SPI工作模式1
#define MODE2 2 // SPI工作模式2
#define MODE3 3 // SPI工作模式3
#define MODE4 4 // SPI工作模式4

/*定义SPI需要使用的片选引脚*/
#define SPI_CS 1
#define SPI_CS2 1
#define SPI_CS3 1

/*定义SPI使用的方向*/
#define SPI_MOSI 1
#define SPI_MISO 0

#if (CPOL == 0 && CPHA == 0)
#define SPI_MODE MODE1 // CPOL=0,CPHA=0
#elif (CPOL == 0 && CPHA == 1)
#define SPI_MODE MODE2 // CPOL=0,CPHA=1
#elif (CPOL == 1 && CPHA == 0)
#define SPI_MODE MODE3 // CPOL=1,CPHA=0
#elif (CPOL == 1 && CPHA == 1)
#define SPI_MODE MODE4 // CPOL=1,CPHA=1
#endif

/**
 * @brief 软件 SPI 底层 API 结构体
 * @note 用于软件 SPI 的底层实现，保持原有接口以兼容现有代码
 */
typedef struct df_soft_spi_struct
{
    void (*gpio_init)(void);     // SPI GPIO端口初始化函数
    void (*sck)(uint8_t state);  // SCK线控制函数
    void (*mosi)(uint8_t state); // MOSI线控制函数
    uint8_t (*miso)(void);       // 读取MISO线状态
    void (*cs)(uint8_t state);   // CS线控制函数
    void (*cs2)(uint8_t state);  // CS2线控制函数
    void (*cs3)(uint8_t state);  // CS3线控制函数
} df_soft_spi_t;

/**
 * @brief SPI 传输参数结构体
 * @note 用于统一接口的数据传输
 */
typedef struct df_spi_xfer_struct
{
    uint8_t *tx_buf; // 发送缓冲区
    uint8_t *rx_buf; // 接收缓冲区
    uint16_t len;    // 数据长度
    uint8_t cs_num;  // 片选编号 (0=CS, 1=CS2, 2=CS3)
} df_spi_xfer_t;

/**
 * @brief SPI 设备统一接口结构体
 * @note 所有接口统一使用 int (*)(df_arg_t) 类型
 *       - init: 初始化SPI，传参 arg_null
 *       - deinit: 关闭SPI，传参 arg_null
 *       - transfer: 数据交换，传参 arg_ptr(df_spi_xfer_t*)
 *       - send: 发送数据，传参 arg_ptr(df_spi_xfer_t*)
 *       - receive: 接收数据，传参 arg_ptr(df_spi_xfer_t*)
 *       - cs_ctrl: 片选控制，传参 arg_s32(0/1) 低/高电平
 */
typedef struct df_spi_struct
{
    bool init_flag;            // SPI初始化标志
    uint8_t num;               // SPI编号
    char *name;                // SPI名称
    int (*init)(df_arg_t);     // 初始化SPI，传参 arg_null
    int (*deinit)(df_arg_t);   // 关闭SPI，传参 arg_null
    int (*transfer)(df_arg_t); // 数据交换，传参 arg_ptr(df_spi_xfer_t*)
    int (*send)(df_arg_t);     // 发送数据，传参 arg_ptr(df_spi_xfer_t*)
    int (*receive)(df_arg_t);  // 接收数据，传参 arg_ptr(df_spi_xfer_t*)
    int (*cs_ctrl)(df_arg_t);  // 片选控制，传参 arg_s32(0/1)
    df_soft_spi_t *soft_spi;   // 软件SPI底层接口（可选）
} df_spi_t;

// 软件 SPI 底层操作函数（保持兼容性）
#if (MISO && MOSI) || (MISO && !MOSI)
uint8_t *Soft_SPI_SwapData(df_soft_spi_t *spi_Dev, uint8_t *data, uint16_t len); // SPI数据交换函数
uint8_t Soft_SPI_SwapBytes(df_soft_spi_t *spi_Dev, uint8_t data);
uint8_t Soft_SPI_RecvByte(df_soft_spi_t *spi_Dev, uint8_t Byte);
#else
void Soft_SPI_Init(df_soft_spi_t *spi_dev);
void Soft_SPI_SendByte(df_soft_spi_t *spi_Dev, uint8_t Byte);
void Soft_SPI_SwapBytes(df_soft_spi_t *spi_Dev, uint8_t data);
void Soft_SPI_SwapData(df_soft_spi_t *spi_Dev, uint8_t *data, uint16_t len);
#endif

#endif /* __DF_SPI_H_ */