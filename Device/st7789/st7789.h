/**
 * @file st7789.h
 * @brief ST7789 LCD显示屏驱动头文件
 * @note 使用 device_hal.h 统一 HAL 接口，支持 SPI 通信方式
 */

#ifndef __ST7789_H_
#define __ST7789_H_

#include <config.h>
#ifdef USE_DEVICE_ST7789

#include <stdint.h>
#include <stdbool.h>
#include <device_hal.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*============================ 屏幕参数 ============================*/
#define ST7789_WIDTH 240
#define ST7789_HEIGHT 240

/*============================ 颜色定义 (RGB565) ============================*/
#define ST7789_BLACK 0x0000
#define ST7789_BLUE 0x001F
#define ST7789_RED 0xF800
#define ST7789_GREEN 0x07E0
#define ST7789_CYAN 0x07FF
#define ST7789_MAGENTA 0xF81F
#define ST7789_YELLOW 0xFFE0
#define ST7789_WHITE 0xFFFF
#define ST7789_ORANGE 0xFD20
#define ST7789_GRAY 0x8410

    /*============================ HAL接口声明 ============================*/

    /**
     * @brief ST7789 SPI HAL接口实例
     */
    extern device_spi_hal_t *st7789_spi_hal;

    /**
     * @brief ST7789 扩展控制引脚回调
     */
    typedef struct
    {
        void (*dc_control)(bool level);  /* DC引脚控制: true=Data, false=Command */
        void (*res_control)(bool level); /* RES引脚控制: true=Work, false=Reset */
        void (*blk_control)(bool level); /* 背光控制: true=On, false=Off (可选) */
    } st7789_gpio_t;

    /**
     * @brief 初始化ST7789并绑定SPI HAL接口
     * @param hal SPI HAL接口指针
     * @param gpio GPIO控制结构体指针
     * @return 0-成功，非0-失败
     */
    int ST7789_Init_HAL_SPI(device_spi_hal_t *hal, st7789_gpio_t *gpio);

    /**
     * @brief 初始化ST7789并绑定HAL接口（兼容旧接口）
     * @param hal SPI HAL接口指针
     * @return 0-成功，非0-失败
     */
    int ST7789_Init_HAL(device_spi_hal_t *hal);

    /*============================ 初始化函数 ============================*/

    /**
     * @brief 初始化ST7789
     * @return 0-成功，非0-失败
     */
    uint8_t ST7789_Init(void);

    /**
     * @brief 检测ST7789设备是否存在
     * @return 0-设备存在，非0-设备不存在
     */
    uint8_t ST7789_CheckDevice(void);

    /*============================ 显示控制函数 ============================*/

    /**
     * @brief 全屏填充颜色
     * @param color RGB565颜色值
     */
    void ST7789_Clear(uint16_t color);

    /**
     * @brief 填充矩形区域
     * @param x 左上角X坐标
     * @param y 左上角Y坐标
     * @param w 宽度
     * @param h 高度
     * @param color RGB565颜色值
     */
    void ST7789_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

    /**
     * @brief 设置像素点
     * @param x X坐标
     * @param y Y坐标
     * @param color RGB565颜色值
     */
    void ST7789_SetPixel(uint16_t x, uint16_t y, uint16_t color);

    /**
     * @brief 显示图像
     * @param x 左上角X坐标
     * @param y 左上角Y坐标
     * @param w 宽度
     * @param h 高度
     * @param data RGB565图像数据
     */
    void ST7789_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data);

    /**
     * @brief 设置显示方向
     * @param rotation 旋转方向 (0-3)
     */
    void ST7789_SetRotation(uint8_t rotation);

    /**
     * @brief 背光控制
     * @param on true-开启, false-关闭
     */
    void ST7789_Backlight(bool on);

    /**
     * @brief 设置显示窗口
     * @param x1 起始X坐标
     * @param y1 起始Y坐标
     * @param x2 结束X坐标
     * @param y2 结束Y坐标
     */
    void ST7789_SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

#ifdef __cplusplus
}
#endif

#endif /* USE_DEVICE_ST7789 */
#endif /* __ST7789_H_ */
