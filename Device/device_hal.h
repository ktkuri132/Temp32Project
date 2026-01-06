/**
 * @file    device_hal.h
 * @brief   设备驱动硬件抽象层接口
 * @details 定义设备驱动的底层I2C/SPI通信接口，使用结构体函数指针实现
 *          替代原有的宏函数方式，提供更灵活的硬件抽象
 * @version 2.0
 * @date    2026-01-02
 */

#ifndef __DEVICE_HAL_H__
#define __DEVICE_HAL_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /*============================ I2C接口定义 ============================*/

    /**
     * @brief I2C硬件抽象层接口结构体
     * @note  所有函数指针必须在使用前初始化
     */
    typedef struct
    {
        /**
         * @brief 写入单个寄存器
         * @param dev_addr  设备I2C地址（7位地址左移1位）
         * @param reg_addr  寄存器地址
         * @param data      要写入的数据
         * @return 0-成功，非0-失败
         */
        int (*write_byte)(uint8_t dev_addr, uint8_t reg_addr, uint8_t data);

        /**
         * @brief 读取单个寄存器
         * @param dev_addr  设备I2C地址
         * @param reg_addr  寄存器地址
         * @param data      数据缓冲区指针
         * @return 0-成功，非0-失败
         */
        int (*read_byte)(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data);

        /**
         * @brief 连续读取多个寄存器
         * @param dev_addr  设备I2C地址
         * @param reg_addr  起始寄存器地址
         * @param len       读取长度
         * @param buf       数据缓冲区
         * @return 0-成功，非0-失败
         */
        int (*read_bytes)(uint8_t dev_addr, uint8_t reg_addr, uint8_t len, uint8_t *buf);

        /**
         * @brief 连续写入多个寄存器
         * @param dev_addr  设备I2C地址
         * @param reg_addr  起始寄存器地址
         * @param len       写入长度
         * @param buf       数据缓冲区
         * @return 0-成功，非0-失败
         */
        int (*write_bytes)(uint8_t dev_addr, uint8_t reg_addr, uint8_t len, const uint8_t *buf);

        /**
         * @brief 延时函数（毫秒）
         * @param ms  延时时间（毫秒）
         */
        void (*delay_ms)(uint32_t ms);

        /**
         * @brief 延时函数（微秒）
         * @param us  延时时间（微秒）
         */
        void (*delay_us)(uint32_t us);

        /**
         * @brief 初始化标志
         */
        bool initialized;

        /**
         * @brief 用户数据指针（可选，用于存储底层驱动上下文）
         */
        void *user_data;

    } device_i2c_hal_t;

    /*============================ SPI接口定义 ============================*/

    /**
     * @brief SPI硬件抽象层接口结构体
     * @note  所有函数指针必须在使用前初始化
     */
    typedef struct
    {
        /**
         * @brief 片选使能
         * @param enable  true-片选有效，false-片选无效
         */
        void (*cs_control)(uint8_t enable);

        /**
         * @brief SPI传输单个字节
         * @param tx_data  发送数据
         * @return 接收到的数据
         */
        uint8_t (*transfer_byte)(uint8_t tx_data);

        /**
         * @brief SPI传输多个字节
         * @param tx_buf   发送缓冲区（可为NULL表示只接收）
         * @param rx_buf   接收缓冲区（可为NULL表示只发送）
         * @param len      传输长度
         * @return 0-成功，非0-失败
         */
        int (*transfer_bytes)(const uint8_t *tx_buf, uint8_t *rx_buf, uint16_t len);

        /**
         * @brief 延时函数（毫秒）
         * @param ms  延时时间（毫秒）
         */
        void (*delay_ms)(uint32_t ms);

        /**
         * @brief 延时函数（微秒）
         * @param us  延时时间（微秒）
         */
        void (*delay_us)(uint32_t us);

        /**
         * @brief 初始化标志
         */
        bool initialized;

        /**
         * @brief 用户数据指针（可选）
         */
        void *user_data;

    } device_spi_hal_t;

    /* ===========================综合通讯方式接口定义===================================  */
    typedef struct {
        device_i2c_hal_t i2c; // I2C HAL接口指针
        device_spi_hal_t spi; // SPI HAL接口指针
    } device_interface_hal_t;
/*============================ 辅助宏定义 ============================*/

/**
 * @brief 检查I2C HAL是否已初始化
 */
#define DEVICE_I2C_HAL_CHECK(hal)          \
    do                                     \
    {                                      \
        if (!(hal) || !(hal)->initialized) \
        {                                  \
            return -1; /* HAL未初始化 */   \
        }                                  \
    } while (0)

/**
 * @brief 检查SPI HAL是否已初始化
 */
#define DEVICE_SPI_HAL_CHECK(hal)          \
    do                                     \
    {                                      \
        if (!(hal) || !(hal)->initialized) \
        {                                  \
            return -1; /* HAL未初始化 */   \
        }                                  \
    } while (0)

    /*============================ I2C HAL适配器函数声明 ============================*/

    /**
     * @brief 初始化软件I2C适配器
     * @param hal  I2C HAL结构体指针
     * @param i2c_bus  软件I2C总线结构体指针
     * @return 0-成功，-1-失败
     */
    int device_i2c_hal_init_soft(device_i2c_hal_t *hal, void *i2c_bus);

    /**
     * @brief 初始化硬件I2C适配器
     * @param hal  I2C HAL结构体指针
     * @param hw_i2c  硬件I2C外设指针（如I2C1, I2C2等）
     * @return 0-成功，-1-失败
     */
    int device_i2c_hal_init_hardware(device_i2c_hal_t *hal, void *hw_i2c);

    /*============================ SPI HAL适配器函数声明 ============================*/

    /**
     * @brief 初始化软件SPI适配器
     * @param hal  SPI HAL结构体指针
     * @param spi_bus  软件SPI总线结构体指针
     * @return 0-成功，-1-失败
     */
    int device_spi_hal_init_soft(device_spi_hal_t *hal, void *spi_bus);

    /**
     * @brief 初始化硬件SPI适配器
     * @param hal  SPI HAL结构体指针
     * @param hw_spi  硬件SPI外设指针（如SPI1, SPI2等）
     * @return 0-成功，-1-失败
     */
    int device_spi_hal_init_hardware(device_spi_hal_t *hal, void *hw_spi);

#ifdef __cplusplus
}
#endif

#endif /* __DEVICE_HAL_H__ */
