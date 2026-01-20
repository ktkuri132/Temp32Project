/**
 * @file ssd1306.h
 * @brief SSD1306 OLED显示屏驱动头文件
 * @note 使用 device_hal.h 统一 HAL 接口，支持 I2C/SPI 两种通信方式
 */

#ifndef __SSD1306_H_
#define __SSD1306_H_

#include <config.h>
#ifdef USE_DEVICE_SSD1306

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <device_hal.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*============================ 屏幕参数 ============================*/
#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64

/* SSD1306 I2C地址和寄存器模式 */
#define SSD1306_ADDRESS 0x78
#define SSD1306_Data_Mode 0x40
#define SSD1306_Command_Mode 0x00

    /*============================ HAL接口声明 ============================*/
    /**
     * @brief SSD1306 I2C HAL接口实例
     */
    extern device_i2c_hal_t *ssd1306_i2c_hal;

    /**
     * @brief SSD1306 SPI HAL接口实例
     */
    extern device_spi_hal_t *ssd1306_spi_hal;

    /**
     * @brief 初始化SSD1306并绑定I2C HAL接口
     * @param hal I2C HAL接口指针
     * @return 0-成功，非0-失败
     */
    int SSD1306_Init_HAL_I2C(device_i2c_hal_t *hal);

    /**
     * @brief 初始化SSD1306并绑定SPI HAL接口
     * @param hal SPI HAL接口指针
     * @return 0-成功，非0-失败
     */
    int SSD1306_Init_HAL_SPI(device_spi_hal_t *hal);

    /**
     * @brief 初始化SSD1306并绑定HAL接口（兼容旧接口，默认使用I2C）
     * @param hal I2C HAL接口指针
     * @return 0-成功，非0-失败
     */
    int SSD1306_Init_HAL(device_i2c_hal_t *hal);

/*============================ FontSize参数取值 ============================*/
/*此参数值不仅用于判断，而且用于计算横向字符偏移，默认值为字体像素宽度*/
#define SSD1306_8X16 8
#define SSD1306_6X8 6

/*============================ IsFilled参数数值 ============================*/
#define SSD1306_UNFILLED 0
#define SSD1306_FILLED 1

    /*============================ 初始化函数 ============================*/
    /**
     * @brief 初始化SSD1306
     * @return 0-成功，非0-失败
     */
    uint8_t SSD1306_Init(void);

    /**
     * @brief 检测SSD1306设备是否存在
     * @return 0-设备存在，非0-设备不存在
     */
    uint8_t SSD1306_CheckDevice(void);

    /*============================ 更新函数 ============================*/
    /**
     * @brief 将显存数组更新到SSD1306屏幕
     */
    void SSD1306_Update(void);

    /**
     * @brief 将显存数组部分区域更新到SSD1306屏幕
     */
    void SSD1306_UpdateArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);

    /*============================ 显存控制函数 ============================*/
    /**
     * @brief 清空显存
     */
    void SSD1306_Clear(void);

    /**
     * @brief 清空指定区域显存
     */
    void SSD1306_ClearArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);

    /**
     * @brief 反色显示
     */
    void SSD1306_Reverse(void);

    /**
     * @brief 指定区域反色显示
     */
    void SSD1306_ReverseArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);

    /*============================ 绘图函数 ============================*/
    /**
     * @brief 画点
     */
    void SSD1306_DrawPoint(int16_t X, int16_t Y);

    /**
     * @brief 获取指定点的值
     */
    uint32_t SSD1306_GetPoint(uint16_t X, uint16_t Y);

    /**
     * @brief 设置像素点（兼容统一绘图接口）
     */
    void SSD1306_SetPixel(uint16_t x, uint16_t y, uint32_t color);

    /**
     * @brief 填充矩形区域（兼容统一绘图接口）
     */
    void SSD1306_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color);

    /**
     * @brief 显示图像
     */
    void SSD1306_ShowImage(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image);

#ifdef __cplusplus
}
#endif

#endif /* USE_DEVICE_SSD1306 */
#endif /* __SSD1306_H_ */
