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
#include "device_init.h"
#include "main.h" // 包含 lcd_sh1106 声明
#ifdef USE_DEVICE_SH1106
#include "sh1106/sh1106.h"
#endif
#ifdef USE_DEVICE_SSD1306
#include "ssd1306/ssd1306.h"
#endif
#ifdef USE_DEVICE_ST7789
#include "st7789/st7789.h"
#endif
#ifdef USE_DEVICE_MPU6050
#include "mpu6050/inv_mpu.h"
#endif

extern df_delay_t delay;

/*===========================================================================*/
/*                         SH1106 OLED 设备初始化                             */
/*===========================================================================*/


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

#ifdef USE_DEVICE_MPU6050

typedef enum
{
    DATA = 0,
    OUT_LCD = 1
} mpu6050_arg_t;

int mpu6050_dev_init(df_arg_t arg)
{
    // 后续添加启用mpu6050外部中断的代码
    return Device_MPU6050_Init();
}

int mpu6050_dev_enable(df_arg_t arg)
{
    // 后续添加启用外部中断的代码
    return 0;
}

// 停用 MPU6050 设备
int mpu6050_dev_disable(df_arg_t arg)
{
    // 后续添加停用外部中断的代码
    return 0;
}

int mpu6050_dev_read(df_arg_t arg)
{
    // 读取三轴欧拉角
    df_dev_t *mpu6050 = (df_dev_t *)arg.ptr;
    float *data = (float *)mpu6050->arg.argv[DATA];
    LCD_Handler_t *lcd = (LCD_Handler_t *)mpu6050->arg.argv[OUT_LCD];
    switch (mpu_dmp_get_data(&data[0], &data[1], &data[2]))
    {
    case 1:
        // LOG_E("MPU6050", "mpu6050_dev_read: mpu_dmp_get_data failed!\n");
        return 1;
        break;
    case 2:
        // LOG_W("MPU6050", "mpu6050_dev_read: mpu_dmp_get_data No new data available.\n");
        return 2;
        break;
    default:
        break;
    }
    LCD_Printf(lcd, "%.2f,%.2f,%.2f\n", data[0], data[1], data[2]);
    return 0;
}

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

#endif