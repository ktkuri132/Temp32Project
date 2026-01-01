/**
 * @file    config.h
 * @brief   设备驱动配置文件 (自动生成)
 * @details 本文件由构建系统自动生成，请勿手动编辑
 *          通过修改 tool/project_config.json 来配置设备驱动
 * @warning 此文件会在每次构建时重新生成，手动修改将会丢失
 */

#ifndef __CONFIG_H_
#define __CONFIG_H_

#include <stdint.h>
#include <driver.h>

/*============================ 设备驱动配置 ============================*/

/* I2C通信总线选择 */
#define __SOFTI2C_

#ifdef __SOFTI2C_
#include <i2c/df_iic.h>
extern SIAS i2c1_bus;
#define i2c_Dev i2c1_bus
#endif

/* SH1106 OLED显示屏配置 */
#ifdef USE_DEVICE_SH1106
#endif /* USE_DEVICE_SH1106 */

/* MPU6050 六轴IMU配置 */
#ifdef USE_DEVICE_MPU6050
#endif /* USE_DEVICE_MPU6050 */

/* HMC5883L 三轴磁力计配置 */
#ifdef USE_DEVICE_HMC588
#endif /* USE_DEVICE_HMC588 */

/* BMP280 气压温度传感器配置 */
#ifdef USE_DEVICE_BMP280
#endif /* USE_DEVICE_BMP280 */

#endif /* __CONFIG_H_ */
