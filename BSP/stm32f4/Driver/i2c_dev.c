#include <config.h>
#ifdef USE_DEVICE_SH1106
#include <sh1106/sh1106.h>
#endif
#include <ssd1306/ssd1306.h>
#include <lcd/df_lcd.h>
#include <lcd/df_fonts.h>
#include <df_delay.h>

extern Dt delay;

/**
 * STM32F4 I2C设备驱动
 * SH1106/SSD1306 OLED显示屏驱动
 */

void SH1106_SetPixel(uint16_t x, uint16_t y, uint32_t color)
{
    if (color)
        SH1106_DrawPoint(x, y);
    else
        SH1106_ClearArea(x, y, 1, 1);
}

void SH1106_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color)
{
    (void)color;
    if (x == 0 && y == 0)
    {
        SH1106_Clear();
        return;
    }
    SH1106_ClearArea(x, y, w, h);
}

void SSD1306_SetPixel(uint16_t x, uint16_t y, uint32_t color)
{
    (void)color;
    //    SSD1306_DrawPoint(x, y);
}

int sh1106_dev_init(dev_arg_t arg)
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
    delay.ms(100);
    if (SH1106_Init())
    {
        error("sh1106_dev_init: SH1106_Init failed!\n");
        return -1;
    }
    LCD_Clear(lcd, 0);
    LCD_Printf(lcd, "System Start\n");
    LCD_Printf(lcd, "SH1106 OLED Initialized.\n");
    return 0;
}

int ssd1306_dev_init(dev_arg_t arg)
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
    //    SSD1306_Init();
    LCD_Clear(lcd, 0x00000000);
    LCD_Printf(lcd, "System Start\n");
    LCD_Printf(lcd, "SSD1306 OLED Initialized.\n");
    return 0;
}

uint8_t mpu6050_i2c_write(uint8_t addr, uint8_t reg, uint16_t length, uint8_t *data)
{
    return Soft_IIC_Write_Len(&i2c_Dev, addr, reg, length, data);
}

uint8_t mpu6050_i2c_read(uint8_t addr, uint8_t reg, uint16_t length, uint8_t *data)
{
    return Soft_IIC_Read_Len(&i2c_Dev, addr, reg, length, data);
}
