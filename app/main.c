#include "main.h"
#include <stdint.h>
#include <sh1106.h>
#include <df_display.h>
#include <df_lcd.h>
// 声明flash测试函数
void flash_info_query(void);
uint32_t a = 0;

LCD_Handler_t lcd_oled = {
    .Width = SH1106_WIDTH,
    .Height = SH1106_HEIGHT,
    .SetPixel = SH1106_SetPixel,
    .GetPixel = NULL,    // 可选实现
    .FillRect = NULL,    // 可选实现
    .Update = SH1106_Update,
    .ScrollHard = NULL,  // 可选实现
    .CursorX = 0,
    .CursorY = 0,
    .CurrentFont = NULL, // 可选设置
    .TextColor = 0xFFFFFFFF,
    .BackColor = 0x00000000,
    .TerminalMode = true
};

DisplayDevice_t oled = {
    .Init = SH1106_Init,
    .Clear = NULL, // 由 LCD 框架处理
};

int main()
{
    debug.send(arg_ptr("System Start!\r\n"));
    Device_Registration(Dev_info_poor); // 初始化设备模型
    // MCU_Shell_Init(&Shell, &STM32F103C8T6_Device);  // 初始化Shell
    // 运行flash测试
    // flash_info_query();
    Display.RegisterLCD(&oled, &lcd_oled, "OLED_SH1106"); // 注册LCD显示设备
    Display.Select("OLED_SH1106"); // 选择当前显示设备
    Display.Printf("Hello, World!\n");
    while (1)
    {
        // Task_Switch_Tick_Handler(&Shell_Sysfpoint);
        led.toggle(arg_null);
        adc1.get_value(arg_u32(a));
        delay.ms(500);
    }
    return 0;
}
