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

/*===========================================================================*/
/*                    软件IIC内联函数实现（性能优化）                          */
/*===========================================================================*/

/**
 * @brief 产生IIC起始信号（内联优化）
 */
static inline void Soft_IIC_Start(df_soft_iic_t *i2c)
{
    i2c->sda_out();
    i2c->sda(1);
    i2c->scl(1);
    i2c->sda(0);
    i2c->scl(0);
}

/**
 * @brief 产生IIC停止信号（内联优化）
 */
static inline void Soft_IIC_Stop(df_soft_iic_t *i2c)
{
    i2c->sda_out();
    i2c->scl(0);
    i2c->sda(0);
    i2c->scl(1);
    i2c->sda(1);
}

/**
 * @brief 产生ACK应答（内联优化）
 */
static inline void Soft_IIC_Ack(df_soft_iic_t *i2c)
{
    i2c->scl(0);
    i2c->sda_out();
    i2c->sda(0);
    i2c->scl(1);
    i2c->scl(0);
}

/**
 * @brief 不产生ACK应答（内联优化）
 */
static inline void Soft_IIC_NAck(df_soft_iic_t *i2c)
{
    i2c->scl(0);
    i2c->sda_out();
    i2c->sda(1);
    i2c->scl(1);
    i2c->scl(0);
}

/**
 * @brief 等待应答信号（内联优化）
 * @return 0:成功, 1:失败
 */
static inline uint8_t Soft_IIC_Wait_Ack(df_soft_iic_t *i2c)
{
    uint8_t ucErrTime = 0;
    i2c->sda_in();
    i2c->sda(1);
    i2c->scl(1);
    while (i2c->read_sda())
    {
        ucErrTime++;
        if (ucErrTime > 250)
        {
            Soft_IIC_Stop(i2c);
            return 1;
        }
    }
    i2c->scl(0);
    return 0;
}

/**
 * @brief IIC发送一个字节（内联优化）
 */
static inline void Soft_IIC_Send_Byte(df_soft_iic_t *i2c, uint8_t txd)
{
    i2c->sda_out();
    i2c->scl(0);
    for (uint8_t t = 0; t < 8; t++)
    {
        i2c->sda((txd & 0x80) >> 7);
        txd <<= 1;
        i2c->scl(1);
        i2c->scl(0);
    }
}

/**
 * @brief 读取一个字节（内联优化）
 * @param ack 1:发送ACK, 0:发送NACK
 */
static inline uint8_t Soft_IIC_Receive_Byte(df_soft_iic_t *i2c, uint8_t ack)
{
    uint8_t receive = 0;
    i2c->sda_in();
    for (uint8_t i = 0; i < 8; i++)
    {
        i2c->scl(0);
        i2c->scl(1);
        receive <<= 1;
        if (i2c->read_sda())
            receive++;
    }
    if (!ack)
        Soft_IIC_NAck(i2c);
    else
        Soft_IIC_Ack(i2c);
    return receive;
}

/*===========================================================================*/
/*                         非内联函数声明                                     */
/*===========================================================================*/

// IIC初始化（不需要内联，只调用一次）
void Soft_IIC_Init(df_soft_iic_t *i2c);

// 高级读写函数（包含循环，不适合内联）
uint8_t Soft_IIC_Write_Len(df_soft_iic_t *i2c, uint8_t addr, uint8_t reg,
                           uint8_t len, uint8_t *buf);
uint8_t Soft_IIC_Read_Len(df_soft_iic_t *i2c, uint8_t addr, uint8_t reg, uint8_t len,
                          uint8_t *buf);
uint8_t Soft_IIC_Write_Byte(df_soft_iic_t *i2c, uint8_t addr, uint8_t reg,
                            uint8_t data);
uint8_t Soft_IIC_Read_Byte(df_soft_iic_t *i2c, uint8_t addr,
                           uint8_t reg);
uint8_t Soft_IIC_Check(df_soft_iic_t *i2c, uint8_t addr);

#endif
