/**
 * @file    device_hal.c
 * @brief   设备驱动硬件抽象层实现
 * @details 实现不同I2C/SPI接口的适配器函数
 * @version 2.0
 * @date    2026-01-02
 */

#include "device_hal.h"
#include <config.h>
#include <string.h>

/* 根据配置包含相应的底层驱动 */
#ifdef __SOFTI2C_
#include <i2c/df_iic.h>
extern df_soft_iic_t i2c1_bus; /* 外部软件I2C总线 */
#endif

#include <df_delay.h>

/*============================ 软件I2C适配器实现 ============================*/

#ifdef __SOFTI2C_

/**
 * @brief 软件I2C写单字节适配器
 */
static int soft_i2c_write_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t data)
{
    return Soft_IIC_Write_Byte(&i2c1_bus, dev_addr, reg_addr, data);
}

/**
 * @brief 软件I2C读单字节适配器
 */
static int soft_i2c_read_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data)
{
    if (!data)
        return -1;
    *data = Soft_IIC_Read_Byte(&i2c1_bus, dev_addr, reg_addr);
    return 0;
}

/**
 * @brief 软件I2C连续读取适配器
 */
static int soft_i2c_read_bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t len, uint8_t *buf)
{
    return Soft_IIC_Read_Len(&i2c1_bus, dev_addr, reg_addr, len, buf);
}

/**
 * @brief 软件I2C连续写入适配器
 */
static int soft_i2c_write_bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t len,
                                const uint8_t *buf)
{
    /* 软件I2C暂未实现多字节写入，使用循环单字节写入 */
    for (uint8_t i = 0; i < len; i++)
    {
        if (Soft_IIC_Write_Byte(&i2c1_bus, dev_addr, reg_addr + i, buf[i]) != 0)
        {
            return -1;
        }
    }
    return 0;
}

/**
 * @brief 初始化软件I2C适配器
 */
int device_i2c_hal_init_soft(device_i2c_hal_t *hal, void *i2c_bus)
{
    if (!hal)
        return -1;

    memset(hal, 0, sizeof(device_i2c_hal_t));

    hal->write_byte = soft_i2c_write_byte;
    hal->read_byte = soft_i2c_read_byte;
    hal->read_bytes = soft_i2c_read_bytes;
    hal->write_bytes = soft_i2c_write_bytes;
    hal->delay_ms = delay_ms;
    hal->delay_us = delay_us;
    hal->user_data = i2c_bus;
    hal->initialized = true;

    return 0;
}

#endif /* __SOFTI2C_ */

/*============================ 硬件I2C适配器实现 ============================*/

#ifdef __HARDI2C_

/* TODO: 实现硬件I2C适配器函数 */

/**
 * @brief 硬件I2C写单字节适配器
 */
static int hard_i2c_write_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t data)
{
    /* 使用硬件I2C实现 */
    /* TODO: 根据实际硬件I2C接口实现 */
    return -1; /* 未实现 */
}

/**
 * @brief 硬件I2C读单字节适配器
 */
static int hard_i2c_read_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data)
{
    /* 使用硬件I2C实现 */
    /* TODO: 根据实际硬件I2C接口实现 */
    return -1; /* 未实现 */
}

/**
 * @brief 硬件I2C连续读取适配器
 */
static int hard_i2c_read_bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t len, uint8_t *buf)
{
    /* 使用硬件I2C实现 */
    /* TODO: 根据实际硬件I2C接口实现 */
    return -1; /* 未实现 */
}

/**
 * @brief 硬件I2C连续写入适配器
 */
static int hard_i2c_write_bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t len,
                                const uint8_t *buf)
{
    /* 使用硬件I2C实现 */
    /* TODO: 根据实际硬件I2C接口实现 */
    return -1; /* 未实现 */
}

/**
 * @brief 初始化硬件I2C适配器
 */
int device_i2c_hal_init_hardware(device_i2c_hal_t *hal, void *hw_i2c)
{
    if (!hal)
        return -1;

    memset(hal, 0, sizeof(device_i2c_hal_t));

    hal->write_byte = hard_i2c_write_byte;
    hal->read_byte = hard_i2c_read_byte;
    hal->read_bytes = hard_i2c_read_bytes;
    hal->write_bytes = hard_i2c_write_bytes;
    hal->delay_ms = delay.ms;
    hal->delay_us = delay.us;
    hal->user_data = hw_i2c;
    hal->initialized = true;

    return 0;
}

#endif /* __HARDI2C_ */

/*============================ 软件SPI适配器实现 ============================*/

/* TODO: 根据需要实现软件SPI适配器 */

int device_spi_hal_init_soft(device_spi_hal_t *hal, void *spi_bus)
{
    if (!hal)
        return -1;

    /* TODO: 实现软件SPI初始化 */
    return -1; /* 未实现 */
}

/*============================ 硬件SPI适配器实现 ============================*/

/* TODO: 根据需要实现硬件SPI适配器 */

int device_spi_hal_init_hardware(device_spi_hal_t *hal, void *hw_spi)
{
    if (!hal)
        return -1;

    /* TODO: 实现硬件SPI初始化 */
    return -1; /* 未实现 */
}
