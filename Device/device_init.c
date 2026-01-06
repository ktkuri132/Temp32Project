/**
 * @file    device_init.c
 * @brief   设备驱动初始化
 * @details 负责初始化所有设备的HAL接口
 * @version 2.0
 * @date    2026-01-02
 */

#include "config.h"
#include "device_hal.h"
#include "df_init.h"
/*============================ 全局HAL接口实例 ============================*/

device_interface_hal_t g_device_interface_hal = {
    .i2c = {0},
    .spi = {0}
};

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
    device_i2c_hal_init_soft(&g_device_interface_hal.i2c, i2c1_bus.soft_iic);
#endif

#ifdef __HARDI2C_
    /* TODO: 初始化硬件I2C HAL适配器 */
    // device_i2c_hal_init_hardware(&g_device_interface_hal.i2c, I2C1);
#endif

#ifdef __SOFTSPI_
    Soft_SPI_Init(spi1_bus.soft_spi);
    /* 初始化软件SPI HAL适配器 */
    device_spi_hal_init_soft(&g_device_interface_hal.spi, spi1_bus.soft_spi);
#endif

#ifdef __HARDSPI_
    /* TODO: 初始化硬件SPI HAL适配器 */
    // device_spi_hal_init_hardware(&g_device_interface_hal.spi, SPI1);
#endif
}

// ============ 自动初始化 ============
/**
 * @brief 设备通信框架自动初始化函数
 * @details 在框架初始化时自动调用，初始化I2C通信框架
 * @return 0表示成功
 */
int df_interface_auto_init(void)
{
    // 初始化I2C框架
    Device_HAL_Init();
// I2C框架暂无需特殊初始化，此函数用于日志记录
#ifdef __SOFTI2C_
    LOG_I("IIC", "Soft I2C framework initialized");
#elif __HARDI2C_
    LOG_I("IIC", "Hard I2C framework initialized");
#endif
#ifdef __SOFTSPI_
    LOG_I("SPI", "Soft SPI framework initialized");
#elif __HARDSPI_
    LOG_I("SPI", "Hard SPI framework initialized");
#endif
    return 0;
}

// 将I2C框架初始化注册到DEVICE级别
DF_INIT_EXPORT(df_interface_auto_init, DF_INIT_EXPORT_PREV);

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
    return mpu_dmp_init();
}
#endif

#ifdef USE_DEVICE_SH1106
#include "sh1106/sh1106.h"

/**
 * @brief 初始化SH1106 OLED显示屏
 * @return 0-成功，负值-失败
 */
int Device_SH1106_Init(private_sh1106_t *private_hal)
{
    // 检查参数，接口传递
    int8_t ret = SH1106_Init_HAL(&g_device_interface_hal, private_hal);
    if( ret != 0 )
        return ret;
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

/* 外部GPIO接口（在BSP层定义）*/
extern st7789_gpio_t st7789_gpio;

/**
 * @brief 初始化ST7789 TFT LCD显示屏
 * @return 0-成功，负值-失败
 * @note ST7789使用HAL风格的SPI接口
 */
int Device_ST7789_Init(void)
{
    /* 绑定SPI HAL接口 */
#ifdef __SOFTSPI_
    if (ST7789_Init_HAL_SPI(&g_device_spi_hal, &st7789_gpio) != 0)
    {
        return -1;
    }
#endif

    /* 初始化GPIO引脚 */
    if (st7789_gpio.pin_init)
    {
        st7789_gpio.pin_init();
    }

    /* 初始化ST7789 */
    ST7789_Init();
    return 0;
}
#endif
