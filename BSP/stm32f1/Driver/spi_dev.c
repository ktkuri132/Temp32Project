/**
 * @file spi_dev.c
 * @brief STM32F1 SPI设备驱动
 * @note 使用 SPI总线实现设备通信
 */

#include "driver.h"
#include "spi/df_spi.h"
#include "lcd/df_lcd.h"
#include "df_delay.h"
#include "df_log.h"
#include <config.h>
#include "device_init.h"

#ifdef USE_DEVICE_ST7789
#include "st7789/st7789.h"
#endif

#ifdef USE_DEVICE_SH1106
#include "sh1106/sh1106.h"
#endif

extern df_delay_t delay;

/*===========================================================================*/
/*                         ST7789 SPI 显示屏设备                              */
/*===========================================================================*/

#ifdef USE_DEVICE_ST7789

#define ST7789_DC_PORT F103_GPIOB
#define ST7789_DC_PIN F103_PIN_0
#define ST7789_RES_PORT F103_GPIOA
#define ST7789_RES_PIN F103_PIN_6
#define ST7789_BLK_PORT F103_GPIOB
#define ST7789_BLK_PIN F103_PIN_1

void st7789_pin_init(void)
{
    f103_gpio_init_quick(ST7789_DC_PORT, ST7789_DC_PIN, F103_GPIO_MODE_OUT_PP, F103_GPIO_SPEED_50MHZ);
    f103_gpio_init_quick(ST7789_RES_PORT, ST7789_RES_PIN, F103_GPIO_MODE_OUT_PP, F103_GPIO_SPEED_50MHZ);
    f103_gpio_init_quick(ST7789_BLK_PORT, ST7789_BLK_PIN, F103_GPIO_MODE_OUT_PP, F103_GPIO_SPEED_50MHZ);
}

void st7789_res_set(bool level)
{
    f103_gpio_write(ST7789_RES_PORT, ST7789_RES_PIN, level);
}

void st7789_blk_set(bool level)
{
    f103_gpio_write(ST7789_BLK_PORT, ST7789_BLK_PIN, level);
}

void st7789_dc_set(bool level)
{
    f103_gpio_write(ST7789_DC_PORT, ST7789_DC_PIN, level);
}

st7789_gpio_t st7789_gpio = {
    .pin_init = st7789_pin_init,
    .dc_control = st7789_dc_set,
    .res_control = st7789_res_set,
    .blk_control = st7789_blk_set};

/**
 * @brief ST7789参数索引枚举
 */
typedef enum
{
    ST7789_ARG_LCD = 0, /* LCD句柄 */
    ST7789_ARG_SPI = 1  /* SPI总线 */
} st7789_arg_idx_t;

/**
 * @brief ST7789设备初始化
 * @param arg 传参 arg.ptr = LCD_Handler_t*
 * @return 0成功，其他失败
 */
int st7789_dev_init(df_arg_t arg)
{
    LCD_Handler_t *lcd = (LCD_Handler_t *)arg.ptr;

    if (lcd == NULL)
    {
        LOG_E("ST7789", "st7789_dev_init: lcd handler is NULL!\n");
        return -1;
    }

    if (lcd->SetPixel == NULL)
    {
        LOG_E("ST7789", "st7789_dev_init: lcd SetPixel function is NULL!\n");
        return -1;
    }

    /* 初始化SPI总线 */
    if (spi1_bus.init != NULL && !spi1_bus.init_flag)
    {
        spi1_bus.init(arg_null);
    }

    delay.ms(arg_u32(100)); /* 等待电源稳定 */

    /* 调用ST7789硬件初始化 */
    if (Device_ST7789_Init() != 0)
    {
        LOG_E("ST7789", "st7789_dev_init: ST7789_Init failed!\n");
        return -1;
    }

    LCD_Clear(lcd, 0x00000000); /* 清屏，黑色背景 */
    LCD_Printf(lcd, "System Start\n");
    LCD_Printf(lcd, "ST7789 LCD Initialized.\n");

    LOG_I("ST7789", "ST7789 device initialized successfully\n");
    return 0;
}

/**
 * @brief ST7789写入数据
 * @param arg 传参 arg_ptr(df_spi_xfer_t*)
 * @return 0成功，其他失败
 */
int st7789_dev_write(df_arg_t arg)
{
    df_spi_xfer_t *xfer = (df_spi_xfer_t *)arg.ptr;

    if (xfer == NULL || xfer->tx_buf == NULL)
    {
        LOG_E("ST7789", "st7789_dev_write: invalid xfer param!\n");
        return -1;
    }

    /* 使用SPI发送数据 */
    return spi1_bus.send(arg);
}

/*===========================================================================*/
/*                         ST7789 SPI 读写接口                                */
/*===========================================================================*/

/**
 * @brief ST7789 SPI写入函数（供底层驱动调用）
 * @param data 数据指针
 * @param len 数据长度
 * @return 0成功，其他失败
 */
uint8_t st7789_spi_write(uint8_t *data, uint16_t len)
{
    df_spi_xfer_t xfer = {
        .tx_buf = data,
        .rx_buf = NULL,
        .len = len,
        .cs_num = 0};

    return spi1_bus.send(arg_ptr(&xfer));
}

/**
 * @brief ST7789 SPI写入单字节（供底层驱动调用）
 * @param byte 要发送的字节
 */
void st7789_spi_write_byte(uint8_t byte)
{
    Soft_SPI_SendByte(spi1_bus.soft_spi, byte);
}

#endif

/*===========================================================================*/
/*                      SH1106 SPI OLED 显示屏设备                            */
/*===========================================================================*/

#ifdef USE_DEVICE_SH1106






#endif /* USE_DEVICE_ST7789 */
