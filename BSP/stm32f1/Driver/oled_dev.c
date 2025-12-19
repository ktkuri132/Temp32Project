#include <sh1106/sh1106.h>
#include <ssd1306/ssd1306.h>
#include <lcd/df_lcd.h>
#include <lcd/df_fonts.h>
#include <config.h>
void SH1106_SetPixel(uint16_t x, uint16_t y, uint32_t color)
{
    (void)color; // 未使用参数防止编译警告
    SH1106_DrawPoint(x, y);
}

// void SSD1306_SetPixel(uint16_t x, uint16_t y, uint32_t color){
//     (void)color; // 未使用参数防止编译警告
//     SSD1306_DrawPoint(x, y);
// }

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
    SH1106_Init();
    LCD_SetFont(lcd, &JetBrains_Mono_Font_8x16);
    LCD_Printf(lcd, "System Start\n");
    LCD_Printf(lcd, "SH1106 OLED Initialized.\n");
    LCD_Update(lcd);
    return 0;
}

// int ssd1306_dev_init(dev_arg_t arg)
//{
//     LCD_Handler_t *lcd = (LCD_Handler_t *)arg.ptr;
//     if( lcd == NULL)
//     {
//         error("ssd1306_dev_init: lcd handler is NULL!\n");
//         return -1;
//     }
//     if( lcd->SetPixel == NULL)
//     {
//         error("ssd1306_dev_init: lcd SetPixel function is NULL!\n");
//         return -1;
//     }
//     if( lcd->Width != 128 || lcd->Height != 64)
//     {
//         error("ssd1306_dev_init: lcd size mismatch! Expected 128x64.\n");
//         return -1;
//     }
//     if( lcd->Update == NULL)
//     {
//         error("ssd1306_dev_init: lcd Update function is NULL!\n");
//         return -1;
//     }
//     SSD1306_Init();
//     LCD_SetFont(lcd, &JetBrains_Mono_Font_8x16);
//     LCD_Clear(lcd, 0x00000000); // 清屏，黑色背景
//     LCD_Printf(lcd, "System Start\n");
//     LCD_Printf(lcd, "SSD1306 OLED Initialized.\n");
//     LCD_Update(lcd);
//     return 0;
// }



uint8_t mpu6050_i2c_write(uint8_t addr, uint8_t reg, uint16_t length, uint8_t *data){
    return Soft_IIC_Write_Len(&i2c_dev,addr, reg, length, data);
}


uint8_t mpu6050_i2c_read(uint8_t addr, uint8_t reg, uint16_t length, uint8_t *data){
    return Soft_IIC_Read_Len(&i2c_dev,addr, reg, length, data);
}


