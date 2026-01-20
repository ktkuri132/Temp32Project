/**
 * @file    device_init.h
 * @brief   设备驱动初始化头文件
 * @details 提供统一的设备初始化接口
 * @version 2.0
 * @date    2026-01-02
 */

#ifndef __DEVICE_INIT_H__
#define __DEVICE_INIT_H__
#include "config.h"
#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief 初始化设备驱动HAL接口
     * @note  应在设备驱动初始化前调用
     */
    void Device_HAL_Init(void);

#ifdef USE_DEVICE_BMP280
    /**
     * @brief 初始化BMP280传感器
     * @return 0-成功，负值-失败
     */
    int Device_BMP280_Init(void);
#endif

#ifdef USE_DEVICE_HMC588
    /**
     * @brief 初始化HMC5883L传感器
     * @return 0-成功，负值-失败
     */
    int Device_HMC5883L_Init(void);
#endif

#ifdef USE_DEVICE_MPU6050
    /**
     * @brief 初始化MPU6050传感器
     * @return 0-成功，负值-失败
     */
    int Device_MPU6050_Init(void);
#endif

#ifdef USE_DEVICE_SH1106
    /**
     * @brief 初始化SH1106 OLED显示屏
     * @return 0-成功，负值-失败
     */
    int Device_SH1106_Init(private_sh1106_t *private_hal);
#endif

#ifdef USE_DEVICE_SSD1306
    /**
     * @brief 初始化SSD1306 OLED显示屏
     * @return 0-成功
     */
    int Device_SSD1306_Init(void);
#endif

#ifdef USE_DEVICE_ST7789
    /**
     * @brief 初始化ST7789 TFT LCD显示屏
     * @return 0-成功
     */
    int Device_ST7789_Init(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __DEVICE_INIT_H__ */
