#ifndef __DF_IIC_H
#define __DF_IIC_H
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <dev_frame.h>

/**
 * @brief 软件 IIC 底层 API 结构体
 * @note 用于软件 IIC 的底层实现，保持原有接口以兼容现有代码
 */
typedef struct df_soft_iic_struct
{
    bool init_flag;                 // 软件IIC初始化标志
    void (*gpio_init)(void);        // IIC GPIO端口初始化函数
    void (*delay_us)(uint32_t nus); // 延时函数
    void (*delay_ms)(uint32_t nms); // 延时函数
    void (*scl)(uint8_t state);     // SCL线控制函数
    void (*sda)(uint8_t state);     // SDA线控制函数
    void (*sda_in)(void);           // SDA线设置为输入
    void (*sda_out)(void);          // SDA线设置为输出
    uint8_t (*read_sda)(void);      // 读取SDA线状态
} df_soft_iic_t;

/**
 * @brief IIC 传输参数结构体
 * @note 用于统一接口的数据传输
 */
typedef struct df_iic_xfer_struct
{
    uint8_t dev_addr; // 设备地址
    uint8_t reg_addr; // 寄存器地址
    uint8_t *buf;     // 数据缓冲区
    uint16_t len;     // 数据长度
} df_iic_xfer_t;

/**
 * @brief IIC 设备统一接口结构体
 * @note 所有接口统一使用 int (*)(df_arg_t) 类型
 *       - init: 初始化IIC，传参 arg_null
 *       - deinit: 关闭IIC，传参 arg_null
 *       - write: 写数据，传参 arg_ptr(df_iic_xfer_t*)
 *       - read: 读数据，传参 arg_ptr(df_iic_xfer_t*)
 *       - check: 检测设备，传参 arg_u32(dev_addr)
 */
typedef struct df_iic_struct
{
    bool init_flag;          // IIC初始化标志
    uint8_t num;             // IIC编号
    char *name;              // IIC名称
    int (*init)(df_arg_t);   // 初始化IIC，传参 arg_null
    int (*deinit)(df_arg_t); // 关闭IIC，传参 arg_null
    int (*write)(df_arg_t);  // 写数据，传参 arg_ptr(df_iic_xfer_t*)
    int (*read)(df_arg_t);   // 读数据，传参 arg_ptr(df_iic_xfer_t*)
    int (*check)(df_arg_t);  // 检测设备，传参 arg_u32(dev_addr)
    df_soft_iic_t *soft_iic; // 软件IIC底层接口（可选）
} df_iic_t;

// IIC所有操作函数（保持兼容性）
void Soft_IIC_Init(df_soft_iic_t *i2c);                   // 初始化IIC的IO口
void Soft_IIC_Start(df_soft_iic_t *i2c);                  // 发送IIC开始信号
void Soft_IIC_Stop(df_soft_iic_t *i2c);                   // 发送IIC停止信号
void Soft_IIC_Send_Byte(df_soft_iic_t *i2c, uint8_t txd); // IIC发送一个字节
uint8_t Soft_IIC_Receive_Byte(df_soft_iic_t *i2c,
                              unsigned char ack);  // IIC读取一个字节
uint8_t Soft_IIC_Wait_Ack(df_soft_iic_t *i2c); // IIC等待ACK信号
void Soft_IIC_Ack(df_soft_iic_t *i2c);         // IIC发送ACK信号
void Soft_IIC_NAck(df_soft_iic_t *i2c);        // IIC不发送ACK信号

uint8_t Soft_IIC_Write_Len(df_soft_iic_t *i2c, uint8_t addr, uint8_t reg,
                           uint8_t len, uint8_t *buf); // IIC连续写
uint8_t Soft_IIC_Read_Len(df_soft_iic_t *i2c, uint8_t addr, uint8_t reg, uint8_t len,
                          uint8_t *buf); // IIC连续读
uint8_t Soft_IIC_Write_Byte(df_soft_iic_t *i2c, uint8_t addr, uint8_t reg,
                            uint8_t data); // IIC写一个字节
uint8_t Soft_IIC_Read_Byte(df_soft_iic_t *i2c, uint8_t addr,
                           uint8_t reg); // IIC读一个字节
uint8_t Soft_IIC_Check(df_soft_iic_t *i2c, uint8_t addr);
#endif
