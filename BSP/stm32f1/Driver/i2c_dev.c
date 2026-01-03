/**
 * @file i2c_dev.c
 * @brief STM32F1 I2C设备驱动
 * @note 使用 I2C总线实现设备通信
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

int sh1106_dev_init(df_arg_t arg)
{
    LCD_Handler_t *lcd = (LCD_Handler_t *)arg.ptr;
    if (lcd == NULL)
    {
        LOG_E("SH1106", "sh1106_dev_init: lcd handler is NULL!\n");
        return -1;
    }
    if (lcd->SetPixel == NULL)
    {
        LOG_E("SH1106", "sh1106_dev_init: lcd SetPixel function is NULL!\n");
        return -1;
    }
    if (lcd->Width != 128 || lcd->Height != 64)
    {
        LOG_E("SH1106", "sh1106_dev_init: lcd size mismatch! Expected 128x64.\n");
        return -1;
    }
    if (lcd->Update == NULL)
    {
        LOG_E("SH1106", "sh1106_dev_init: lcd Update function is NULL!\n");
        return -1;
    }
    delay.ms(arg_u32(100)); // 等待电源稳定
    if (Device_SH1106_Init())
    {
        LOG_E("SH1106", "sh1106_dev_init: SH1106_Init failed!\n");
        return -1;
    }
    LCD_Clear(lcd, 0); // 清屏，黑色背景
    LCD_Printf(lcd, "System Start\n");
    LCD_Printf(lcd, "SH1106 OLED Initialized.\n");
    return 0;
}
#endif

/*===========================================================================*/
/*                         SSD1306 OLED 设备初始化                            */
/*===========================================================================*/

#ifdef USE_DEVICE_SSD1306
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

    // SSD1306_Init();
    LCD_Clear(lcd, 0x00000000); // 清屏，黑色背景
    LCD_Printf(lcd, "System Start\n");
    LCD_Printf(lcd, "SSD1306 OLED Initialized.\n");
    return 0;
}
#endif

/*===========================================================================*/
/*                         MPU6050 I2C 读写接口                               */
/*===========================================================================*/

/**
 * @brief MPU6050 I2C写入函数
 * @param addr 设备地址
 * @param reg 寄存器地址
 * @param length 数据长度
 * @param data 数据指针
 * @return 0成功，其他失败
 */
uint8_t mpu6050_i2c_write(uint8_t addr, uint8_t reg, uint16_t length, uint8_t *data)
{
    return Soft_IIC_Write_Len(&i2c_Dev, addr, reg, (uint8_t)length, data);
}

/**
 * @brief MPU6050 I2C读取函数
 * @param addr 设备地址
 * @param reg 寄存器地址
 * @param length 数据长度
 * @param data 数据指针
 * @return 0成功，其他失败
 */
uint8_t mpu6050_i2c_read(uint8_t addr, uint8_t reg, uint16_t length, uint8_t *data)
{
    return Soft_IIC_Read_Len(&i2c_Dev, addr, reg, (uint8_t)length, data);
}
