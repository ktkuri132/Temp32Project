/**
 * @file i2c_dev.c
 * @brief STM32F4 I2C 设备驱动
 * @note 使用 I2C 总线实现设备通信
 */

#include "driver.h"
#include "i2c/df_iic.h"
#include "lcd/df_lcd.h"
#include "df_delay.h"
#include "df_log.h"
#include <config.h>

#ifdef USE_DEVICE_SH1106
#include "sh1106/sh1106.h"
#endif
#ifdef USE_DEVICE_SSD1306
#include "ssd1306/ssd1306.h"
#endif

extern df_delay_t delay;

/*===========================================================================*/
/*                         SH1106 OLED 设备初始化                             */
/*===========================================================================*/

#ifdef USE_DEVICE_SH1106
/**
 * @brief SH1106 OLED 设备初始化
 * @param arg LCD 句柄指针
 * @return 0 成功, -1 失败
 */
int sh1106_dev_init(df_arg_t arg)
{
    LCD_Handler_t *lcd = (LCD_Handler_t *)arg.ptr;
    if (lcd == NULL)
    {
        error("sh1106_dev_init: lcd handler is NULL!\n");
        return -1;
    }
    if (lcd->SetPixel == NULL)
    {
        error("sh1106_dev_init: lcd SetPixel function is NULL!\n");
        return -1;
    }
    if (lcd->Width != 128 || lcd->Height != 64)
    {
        error("sh1106_dev_init: lcd size mismatch! Expected 128x64.\n");
        return -1;
    }
    if (lcd->Update == NULL)
    {
        error("sh1106_dev_init: lcd Update function is NULL!\n");
        return -1;
    }

    delay.ms(arg_u32(100)); /* 等待电源稳定 */
    if (SH1106_Init())
    {
        error("sh1106_dev_init: SH1106_Init failed!\n");
        return -1;
    }
    LCD_Clear(lcd, 0); /* 清屏，黑色背景 */
    LCD_Printf(lcd, "System Start\n");
    LCD_Printf(lcd, "SH1106 OLED Initialized.\n");
    return 0;
}
#endif

/*===========================================================================*/
/*                         SSD1306 OLED 设备初始化                            */
/*===========================================================================*/

#ifdef USE_DEVICE_SSD1306
/**
 * @brief SSD1306 OLED 设备初始化
 * @param arg LCD 句柄指针
 * @return 0 成功, -1 失败
 */
int ssd1306_dev_init(df_arg_t arg)
{
    LCD_Handler_t *lcd = (LCD_Handler_t *)arg.ptr;
    if (lcd == NULL)
    {
        error("ssd1306_dev_init: lcd handler is NULL!\n");
        return -1;
    }
    if (lcd->SetPixel == NULL)
    {
        error("ssd1306_dev_init: lcd SetPixel function is NULL!\n");
        return -1;
    }
    if (lcd->Width != 128 || lcd->Height != 64)
    {
        error("ssd1306_dev_init: lcd size mismatch! Expected 128x64.\n");
        return -1;
    }
    if (lcd->Update == NULL)
    {
        error("ssd1306_dev_init: lcd Update function is NULL!\n");
        return -1;
    }

    /* SSD1306_Init(); */
    LCD_Clear(lcd, 0x00000000); /* 清屏，黑色背景 */
    LCD_Printf(lcd, "System Start\n");
    LCD_Printf(lcd, "SSD1306 OLED Initialized.\n");
    return 0;
}
#endif

/*===========================================================================*/
/*                         MPU6050 I2C 通信接口                               */
/*===========================================================================*/

#ifdef USE_DEVICE_MPU6050
/**
 * @brief MPU6050 I2C 写数据
 */
uint8_t mpu6050_i2c_write(uint8_t addr, uint8_t reg, uint16_t length, uint8_t *data)
{
    return Soft_IIC_Write_Len(&i2c_Dev, addr, reg, length, data);
}

/**
 * @brief MPU6050 I2C 读数据
 */
uint8_t mpu6050_i2c_read(uint8_t addr, uint8_t reg, uint16_t length, uint8_t *data)
{
    return Soft_IIC_Read_Len(&i2c_Dev, addr, reg, length, data);
}
#endif
