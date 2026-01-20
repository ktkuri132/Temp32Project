/**
 * @file f103_i2c.h
 * @brief STM32F103系列I2C公共驱动头文件
 */

#ifndef __F103_I2C_H
#define __F103_I2C_H

#include <stm32f10x.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        F103_I2C1 = 0,
        F103_I2C2,
        F103_I2C_MAX
    } f103_i2c_port_t;

    typedef enum
    {
        F103_I2C_SPEED_100K = 100000,
        F103_I2C_SPEED_400K = 400000
    } f103_i2c_speed_t;

    typedef struct
    {
        f103_i2c_port_t port;
        f103_i2c_speed_t speed;
    } f103_i2c_config_t;

    int f103_i2c_init(const f103_i2c_config_t *config);
    int f103_i2c_init_quick(f103_i2c_port_t port, f103_i2c_speed_t speed);
    int f103_i2c_write(f103_i2c_port_t port, uint8_t addr, const uint8_t *data, uint32_t len);
    int f103_i2c_read(f103_i2c_port_t port, uint8_t addr, uint8_t *data, uint32_t len);
    int f103_i2c_mem_write(f103_i2c_port_t port, uint8_t addr, uint8_t reg, const uint8_t *data, uint32_t len);
    int f103_i2c_mem_read(f103_i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t *data, uint32_t len);
    bool f103_i2c_check_device(f103_i2c_port_t port, uint8_t addr);

#ifdef __cplusplus
}
#endif

#endif /* __F103_I2C_H */
