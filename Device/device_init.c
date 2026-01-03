/**
 * @file    device_init.c
 * @brief   设备驱动初始化
 * @details 负责初始化所有设备的HAL接口
 * @version 2.0
 * @date    2026-01-02
 */

#include "config.h"
#include "device_hal.h"

/*============================ 全局HAL接口实例 ============================*/

#ifdef __SOFTI2C_
/* 软件I2C HAL接口实例 */
device_i2c_hal_t g_device_i2c_hal = {0};
#endif

/*============================ 初始化函数 ============================*/

/**
 * @brief 初始化设备驱动HAL接口
 * @note  应在设备驱动初始化前调用
 */
void Device_HAL_Init(void)
{
#ifdef __SOFTI2C_
    /* 初始化软件I2C总线 */
    Soft_IIC_Init(i2c1_bus.soft_iic);

    /* 初始化软件I2C HAL适配器 */
    device_i2c_hal_init_soft(&g_device_i2c_hal, i2c1_bus.soft_iic);
#endif

#ifdef __HARDI2C_
    /* TODO: 初始化硬件I2C HAL适配器 */
    // device_i2c_hal_init_hardware(&g_device_i2c_hal, I2C1);
#endif
}

/*============================ 设备初始化函数 ============================*/

#ifdef USE_DEVICE_BMP280
#include "bmp280/bmp280.h"

/**
 * @brief 初始化BMP280传感器
 * @return 0-成功，负值-失败
 */
int Device_BMP280_Init(void)
{
    BMP280_BindHAL(&g_device_i2c_hal);
    return BMP280_Init();
}
#endif

#ifdef USE_DEVICE_HMC588
#include "hmc588/hmc588.h"

/**
 * @brief 初始化HMC5883L传感器
 * @return 0-成功，负值-失败
 */
int Device_HMC5883L_Init(void)
{
    HMC5883L_BindHAL(&g_device_i2c_hal);
    return HMC5883L_Init();
}
#endif

#ifdef USE_DEVICE_MPU6050
#include "mpu6050/inv_mpu.h"

/**
 * @brief 初始化MPU6050传感器
 * @return 0-成功，负值-失败
 * @note MPU6050使用旧的I2C接口，不需要HAL绑定
 */
int Device_MPU6050_Init(void)
{
    return mpu_init();
}
#endif

#ifdef USE_DEVICE_SH1106
#include "sh1106/sh1106.h"

/**
 * @brief 初始化SH1106 OLED显示屏
 * @return 0-成功，负值-失败
 */
int Device_SH1106_Init(void)
{
    SH1106_Init_HAL(&g_device_i2c_hal);
    return SH1106_Init();
}
#endif

#ifdef USE_DEVICE_SSD1306
#include "ssd1306/ssd1306.h"

/**
 * @brief 初始化SSD1306 OLED显示屏
 * @return 无返回值
 * @note SSD1306使用旧的I2C/SPI接口，不需要HAL绑定
 */
int Device_SSD1306_Init(void)
{
    SSD1306_Init();
    return 0;
}
#endif

#ifdef USE_DEVICE_ST7789
#include "st7789/st7789.h"

/**
 * @brief 初始化ST7789 TFT LCD显示屏
 * @return 无返回值
 * @note ST7789使用HAL风格的SPI接口，需要在调用前注册IO接口
 */
int Device_ST7789_Init(void)
{
    ST7789_Init();
    return 0;
}
#endif

/**
 * @brief 初始化所有已启用的设备
 * @note  会自动初始化config.h中定义的所有设备
 */
void Device_Init_All(void)
{
    /* 首先初始化HAL接口 */
    Device_HAL_Init();

#ifdef USE_DEVICE_BMP280
    Device_BMP280_Init();
#endif

#ifdef USE_DEVICE_HMC588
    Device_HMC5883L_Init();
#endif

#ifdef USE_DEVICE_MPU6050
    Device_MPU6050_Init();
#endif

#ifdef USE_DEVICE_SH1106
    Device_SH1106_Init();
#endif

#ifdef USE_DEVICE_SSD1306
    Device_SSD1306_Init();
#endif

#ifdef USE_DEVICE_ST7789
    Device_ST7789_Init();
#endif
}
