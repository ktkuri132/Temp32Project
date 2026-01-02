/**
 * @file vet6_i2c.h
 * @brief STM32F407VET6 I2C板级驱动头文件
 * @details 提供I2C1-I2C3的初始化和操作API
 *          支持硬件I2C和软件模拟I2C
 *
 * 默认引脚配置:
 *   I2C1: PB6(SCL), PB7(SDA) 或 PB8(SCL), PB9(SDA)
 *   I2C2: PB10(SCL), PB11(SDA)
 *   I2C3: PA8(SCL), PC9(SDA)
 */

#ifndef __VET6_I2C_H
#define __VET6_I2C_H

#include <stm32f4xx.h>
#include <stdint.h>
#include <stdbool.h>
#include "vet6_gpio.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /*===========================================================================*/
    /*                              类型定义                                      */
    /*===========================================================================*/

    /**
     * @brief 硬件I2C编号枚举
     */
    typedef enum
    {
        VET6_I2C1 = 0,
        VET6_I2C2,
        VET6_I2C3,
        VET6_I2C_MAX
    } vet6_i2c_t;

    /**
     * @brief I2C速度模式
     */
    typedef enum
    {
        VET6_I2C_SPEED_STANDARD = 100000,  // 100kHz
        VET6_I2C_SPEED_FAST = 400000,      // 400kHz
        VET6_I2C_SPEED_FAST_PLUS = 1000000 // 1MHz (部分器件支持)
    } vet6_i2c_speed_t;

    /**
     * @brief I2C引脚映射
     */
    typedef enum
    {
        VET6_I2C1_PINS_PB6_PB7 = 0, // I2C1: PB6(SCL), PB7(SDA)
        VET6_I2C1_PINS_PB8_PB9,     // I2C1: PB8(SCL), PB9(SDA)
        VET6_I2C2_PINS_PB10_PB11,   // I2C2: PB10(SCL), PB11(SDA)
        VET6_I2C3_PINS_PA8_PC9      // I2C3: PA8(SCL), PC9(SDA)
    } vet6_i2c_pinmap_t;

    /**
     * @brief I2C地址模式
     */
    typedef enum
    {
        VET6_I2C_ADDR_7BIT = 0,
        VET6_I2C_ADDR_10BIT = 1
    } vet6_i2c_addr_mode_t;

    /**
     * @brief 硬件I2C配置结构体
     */
    typedef struct
    {
        vet6_i2c_t i2c;                 // I2C编号
        uint32_t speed;                 // I2C速度 (Hz)
        vet6_i2c_addr_mode_t addr_mode; // 地址模式
        vet6_i2c_pinmap_t pinmap;       // 引脚映射
        bool use_dma;                   // 是否使用DMA
    } vet6_i2c_config_t;

    /**
     * @brief 硬件I2C句柄结构体
     */
    typedef struct
    {
        I2C_TypeDef *instance;    // I2C外设指针
        vet6_i2c_config_t config; // 配置信息
        bool initialized;         // 初始化标志
    } vet6_i2c_handle_t;

    /**
     * @brief 软件I2C配置结构体
     */
    typedef struct
    {
        vet6_gpio_id_t scl_pin; // SCL引脚
        vet6_gpio_id_t sda_pin; // SDA引脚
        uint32_t delay_us;      // 位延时(微秒)
    } vet6_soft_i2c_config_t;

    /**
     * @brief 软件I2C句柄结构体
     */
    typedef struct
    {
        vet6_gpio_id_t scl_pin; // SCL引脚
        vet6_gpio_id_t sda_pin; // SDA引脚
        uint32_t delay_us;      // 位延时(微秒)
        bool initialized;       // 初始化标志
    } vet6_soft_i2c_handle_t;

/*===========================================================================*/
/*                              宏定义                                        */
/*===========================================================================*/

/* I2C时钟 */
#define VET6_I2C_APB1_CLK 42000000UL // APB1时钟

/* 超时时间 */
#define VET6_I2C_TIMEOUT 10000

/* I2C标志 */
#define VET6_I2C_FLAG_BUSY I2C_SR2_BUSY
#define VET6_I2C_FLAG_SB I2C_SR1_SB
#define VET6_I2C_FLAG_ADDR I2C_SR1_ADDR
#define VET6_I2C_FLAG_BTF I2C_SR1_BTF
#define VET6_I2C_FLAG_TXE I2C_SR1_TXE
#define VET6_I2C_FLAG_RXNE I2C_SR1_RXNE
#define VET6_I2C_FLAG_AF I2C_SR1_AF

    /*===========================================================================*/
    /*                              硬件I2C API                                   */
    /*===========================================================================*/

    /**
     * @brief 使能I2C时钟
     * @param i2c I2C编号
     */
    void vet6_i2c_clk_enable(vet6_i2c_t i2c);

    /**
     * @brief 禁用I2C时钟
     * @param i2c I2C编号
     */
    void vet6_i2c_clk_disable(vet6_i2c_t i2c);

    /**
     * @brief 获取I2C外设基地址
     * @param i2c I2C编号
     * @return I2C外设指针
     */
    I2C_TypeDef *vet6_i2c_get_base(vet6_i2c_t i2c);

    /**
     * @brief 初始化I2C引脚
     * @param pinmap 引脚映射
     * @return 0:成功, -1:失败
     */
    int vet6_i2c_pins_init(vet6_i2c_pinmap_t pinmap);

    /**
     * @brief 初始化硬件I2C
     * @param handle I2C句柄指针
     * @param config 配置结构体指针
     * @return 0:成功, -1:失败
     */
    int vet6_i2c_init(vet6_i2c_handle_t *handle, const vet6_i2c_config_t *config);

    /**
     * @brief 快速初始化I2C为主机模式
     * @param handle I2C句柄指针
     * @param i2c I2C编号
     * @param speed 速度(Hz)
     * @param pinmap 引脚映射
     * @return 0:成功, -1:失败
     */
    int vet6_i2c_init_master(vet6_i2c_handle_t *handle, vet6_i2c_t i2c,
                             uint32_t speed, vet6_i2c_pinmap_t pinmap);

    /**
     * @brief 反初始化I2C
     * @param handle I2C句柄指针
     */
    void vet6_i2c_deinit(vet6_i2c_handle_t *handle);

    /**
     * @brief I2C发送起始条件
     * @param handle I2C句柄指针
     * @return 0:成功, -1:失败
     */
    int vet6_i2c_start(vet6_i2c_handle_t *handle);

    /**
     * @brief I2C发送停止条件
     * @param handle I2C句柄指针
     */
    void vet6_i2c_stop(vet6_i2c_handle_t *handle);

    /**
     * @brief I2C发送地址
     * @param handle I2C句柄指针
     * @param addr 7位设备地址
     * @param dir 方向 (0:写, 1:读)
     * @return 0:成功, -1:失败/无应答
     */
    int vet6_i2c_send_addr(vet6_i2c_handle_t *handle, uint8_t addr, uint8_t dir);

    /**
     * @brief I2C发送单字节
     * @param handle I2C句柄指针
     * @param data 发送数据
     * @return 0:成功, -1:失败
     */
    int vet6_i2c_send_byte(vet6_i2c_handle_t *handle, uint8_t data);

    /**
     * @brief I2C接收单字节(带ACK)
     * @param handle I2C句柄指针
     * @param data 接收数据指针
     * @return 0:成功, -1:失败
     */
    int vet6_i2c_receive_byte_ack(vet6_i2c_handle_t *handle, uint8_t *data);

    /**
     * @brief I2C接收单字节(带NACK)
     * @param handle I2C句柄指针
     * @param data 接收数据指针
     * @return 0:成功, -1:失败
     */
    int vet6_i2c_receive_byte_nack(vet6_i2c_handle_t *handle, uint8_t *data);

    /**
     * @brief I2C写入数据到设备
     * @param handle I2C句柄指针
     * @param dev_addr 7位设备地址
     * @param data 发送数据指针
     * @param len 数据长度
     * @return 0:成功, -1:失败
     */
    int vet6_i2c_write(vet6_i2c_handle_t *handle, uint8_t dev_addr,
                       const uint8_t *data, uint16_t len);

    /**
     * @brief I2C从设备读取数据
     * @param handle I2C句柄指针
     * @param dev_addr 7位设备地址
     * @param data 接收数据指针
     * @param len 数据长度
     * @return 0:成功, -1:失败
     */
    int vet6_i2c_read(vet6_i2c_handle_t *handle, uint8_t dev_addr,
                      uint8_t *data, uint16_t len);

    /**
     * @brief I2C写入数据到设备寄存器
     * @param handle I2C句柄指针
     * @param dev_addr 7位设备地址
     * @param reg_addr 寄存器地址
     * @param data 发送数据指针
     * @param len 数据长度
     * @return 0:成功, -1:失败
     */
    int vet6_i2c_mem_write(vet6_i2c_handle_t *handle, uint8_t dev_addr,
                           uint8_t reg_addr, const uint8_t *data, uint16_t len);

    /**
     * @brief I2C从设备寄存器读取数据
     * @param handle I2C句柄指针
     * @param dev_addr 7位设备地址
     * @param reg_addr 寄存器地址
     * @param data 接收数据指针
     * @param len 数据长度
     * @return 0:成功, -1:失败
     */
    int vet6_i2c_mem_read(vet6_i2c_handle_t *handle, uint8_t dev_addr,
                          uint8_t reg_addr, uint8_t *data, uint16_t len);

    /**
     * @brief 检测I2C设备是否存在
     * @param handle I2C句柄指针
     * @param dev_addr 7位设备地址
     * @return true:存在, false:不存在
     */
    bool vet6_i2c_device_ready(vet6_i2c_handle_t *handle, uint8_t dev_addr);

    /**
     * @brief 扫描I2C总线上的设备
     * @param handle I2C句柄指针
     * @param addr_list 存储发现的地址数组
     * @param max_count 数组最大容量
     * @return 发现的设备数量
     */
    uint8_t vet6_i2c_scan(vet6_i2c_handle_t *handle, uint8_t *addr_list, uint8_t max_count);

    /**
     * @brief 复位I2C总线
     * @param handle I2C句柄指针
     */
    void vet6_i2c_reset(vet6_i2c_handle_t *handle);

    /*===========================================================================*/
    /*                              软件I2C API                                   */
    /*===========================================================================*/

    /**
     * @brief 初始化软件I2C
     * @param handle 软件I2C句柄指针
     * @param config 配置结构体指针
     * @return 0:成功, -1:失败
     */
    int vet6_soft_i2c_init(vet6_soft_i2c_handle_t *handle, const vet6_soft_i2c_config_t *config);

    /**
     * @brief 快速初始化软件I2C
     * @param handle 软件I2C句柄指针
     * @param scl_pin SCL引脚
     * @param sda_pin SDA引脚
     * @return 0:成功, -1:失败
     */
    int vet6_soft_i2c_init_quick(vet6_soft_i2c_handle_t *handle,
                                 vet6_gpio_id_t scl_pin, vet6_gpio_id_t sda_pin);

    /**
     * @brief 软件I2C发送起始条件
     * @param handle 软件I2C句柄指针
     */
    void vet6_soft_i2c_start(vet6_soft_i2c_handle_t *handle);

    /**
     * @brief 软件I2C发送停止条件
     * @param handle 软件I2C句柄指针
     */
    void vet6_soft_i2c_stop(vet6_soft_i2c_handle_t *handle);

    /**
     * @brief 软件I2C发送单字节
     * @param handle 软件I2C句柄指针
     * @param data 发送数据
     * @return ACK状态 (0:ACK, 1:NACK)
     */
    uint8_t vet6_soft_i2c_send_byte(vet6_soft_i2c_handle_t *handle, uint8_t data);

    /**
     * @brief 软件I2C接收单字节
     * @param handle 软件I2C句柄指针
     * @param ack 是否发送ACK (true:ACK, false:NACK)
     * @return 接收到的数据
     */
    uint8_t vet6_soft_i2c_receive_byte(vet6_soft_i2c_handle_t *handle, bool ack);

    /**
     * @brief 软件I2C写入数据到设备寄存器
     * @param handle 软件I2C句柄指针
     * @param dev_addr 7位设备地址
     * @param reg_addr 寄存器地址
     * @param data 发送数据指针
     * @param len 数据长度
     * @return 0:成功, -1:失败
     */
    int vet6_soft_i2c_mem_write(vet6_soft_i2c_handle_t *handle, uint8_t dev_addr,
                                uint8_t reg_addr, const uint8_t *data, uint16_t len);

    /**
     * @brief 软件I2C从设备寄存器读取数据
     * @param handle 软件I2C句柄指针
     * @param dev_addr 7位设备地址
     * @param reg_addr 寄存器地址
     * @param data 接收数据指针
     * @param len 数据长度
     * @return 0:成功, -1:失败
     */
    int vet6_soft_i2c_mem_read(vet6_soft_i2c_handle_t *handle, uint8_t dev_addr,
                               uint8_t reg_addr, uint8_t *data, uint16_t len);

    /**
     * @brief 软件I2C检测设备
     * @param handle 软件I2C句柄指针
     * @param dev_addr 7位设备地址
     * @return true:存在, false:不存在
     */
    bool vet6_soft_i2c_device_ready(vet6_soft_i2c_handle_t *handle, uint8_t dev_addr);

#ifdef __cplusplus
}
#endif

#endif /* __VET6_I2C_H */
