#include "main.h"
#include <stdint.h>
#include <sh1106.h>
#include <df_display.h>
#include <df_lcd.h>

// 声明flash测试函数
void flash_info_query(void);
uint32_t a = 0;

LCD_Font_t SH1106_6x8 = {
    .table = (uint8_t *)SH1106_F6x8,
    .Width = 6,
    .Height = 8};

LCD_Handler_t lcd_oled = {
    .Width = SH1106_WIDTH,
    .Height = SH1106_HEIGHT,
    .SetPixel = SH1106_SetPixel,
    .GetPixel = NULL, // 可选实现
    .FillRect = NULL, // 可选实现
    .Update = SH1106_Update,
    .ScrollHard = NULL, // 可选实现
    .CursorX = 0,
    .CursorY = 0,
    .CurrentFont = &Font6x8, // 可选设置
    .TextColor = 0xFFFFFFFF,
    .BackColor = 0x00000000,
    .TerminalMode = true};

// DisplayDevice_t oled = {
//     .Init = SH1106_Init,
//     .Clear = NULL, // 由 LCD 框架处理
// };

const uint8_t strings[] = {
    0B10000000,
    0B11000000,
    0B11100000,
    0B11110000,
    0B11111000,
    0B11111100,
    0B11111110,
    0B11111111
};

int main()
{

    Device_Registration(Dev_info_poor); // 初始化设备模型
    debug.send(arg_ptr("System Start!\r\n"));
    SH1106_Init(); // 初始化SH1106
    // MCU_Shell_Init(&Shell, &STM32F103C8T6_Device);  // 初始化Shell
    // 运行flash测试
    // flash_info_query();
    // Display.RegisterLCD(&oled, &lcd_oled, "OLED_SH1106"); // 注册LCD显示设备
    // Display.Select("OLED_SH1106"); // 选择当前显示设备
    // Display.Printf("Hello, World!\n");
    // SH1106_Printf(0, 0, SH1106_8X16, "Hello, World!");
    // SH1106_Update(); // 更新显示内容
    LCD_SetFont(&lcd_oled, &Font6x8);
    // LCD_WriteByte(&lcd_oled, 0, 0, strings[0], true);
    // LCD_WriteByte(&lcd_oled, 0, 1, strings[1], true);
    // LCD_WriteByte(&lcd_oled, 0, 2, strings[2], true);
    // LCD_WriteByte(&lcd_oled, 0, 3, strings[3], true);
    // LCD_WriteByte(&lcd_oled, 0, 4, strings[4], true);
    // LCD_WriteByte(&lcd_oled, 0, 5, strings[5], true);
    // LCD_WriteByte(&lcd_oled, 0, 6, strings[6], true);
    // LCD_WriteByte(&lcd_oled, 0, 7, strings[7], true);

    LCD_ShowImg(&lcd_oled, 0, 0, 8, 1, strings);
    LCD_Update(&lcd_oled); // 更新显示内容
    while (1)
    {

        // Task_Switch_Tick_Handler(&Shell_Sysfpoint);
        led.toggle(arg_null);
        // adc1.get_value(arg_u32(a));
        delay.ms(500);
    }
    return 0;
}
