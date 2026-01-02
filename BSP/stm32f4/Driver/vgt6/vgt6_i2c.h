/**
 * @file vgt6_i2c.h
 * @brief STM32F407VGT6 I2C驱动 - 复用VET6驱动
 */

#ifndef __VGT6_I2C_H
#define __VGT6_I2C_H

#include "../vet6/vet6_i2c.h"

/* 类型别名 */
typedef vet6_i2c_t vgt6_i2c_t;
typedef vet6_i2c_speed_t vgt6_i2c_speed_t;
typedef vet6_i2c_config_t vgt6_i2c_config_t;
typedef vet6_i2c_handle_t vgt6_i2c_handle_t;
typedef vet6_soft_i2c_handle_t vgt6_soft_i2c_handle_t;

/* 枚举别名 */
#define VGT6_I2C1 VET6_I2C1
#define VGT6_I2C2 VET6_I2C2
#define VGT6_I2C3 VET6_I2C3
#define VGT6_I2C_MAX VET6_I2C_MAX

#define VGT6_I2C_SPEED_100K VET6_I2C_SPEED_100K
#define VGT6_I2C_SPEED_400K VET6_I2C_SPEED_400K

/* 函数别名 - 硬件I2C */
#define vgt6_i2c_clk_enable vet6_i2c_clk_enable
#define vgt6_i2c_clk_disable vet6_i2c_clk_disable
#define vgt6_i2c_init vet6_i2c_init
#define vgt6_i2c_init_simple vet6_i2c_init_simple
#define vgt6_i2c_deinit vet6_i2c_deinit
#define vgt6_i2c_enable vet6_i2c_enable
#define vgt6_i2c_disable vet6_i2c_disable
#define vgt6_i2c_write vet6_i2c_write
#define vgt6_i2c_read vet6_i2c_read
#define vgt6_i2c_mem_write vet6_i2c_mem_write
#define vgt6_i2c_mem_read vet6_i2c_mem_read

/* 函数别名 - 软件I2C */
#define vgt6_soft_i2c_init vet6_soft_i2c_init
#define vgt6_soft_i2c_init_quick vet6_soft_i2c_init_quick
#define vgt6_soft_i2c_start vet6_soft_i2c_start
#define vgt6_soft_i2c_stop vet6_soft_i2c_stop
#define vgt6_soft_i2c_write vet6_soft_i2c_write
#define vgt6_soft_i2c_read vet6_soft_i2c_read
#define vgt6_soft_i2c_mem_write vet6_soft_i2c_mem_write
#define vgt6_soft_i2c_mem_read vet6_soft_i2c_mem_read

#endif /* __VGT6_I2C_H */
