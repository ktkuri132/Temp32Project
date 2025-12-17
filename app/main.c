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
    .GetPixel = NULL, // 可选实现
    .FillRect = NULL, // 可选实现
    .Update = SH1106_Update,
    .ScrollHard = NULL, // 可选实现
    .CursorX = 0,
    .CursorY = 0,
    .CurrentFont = &Font8x16, // 可选设置
    .TextColor = 0xFFFFFFFF,
    .BackColor = 0x00000000,
    .TerminalMode = true
};

int main()
{

    Device_Registration(Dev_info_poor); // 初始化设备模型
    debug.send(arg_ptr("System Start!\r\n"));
    SH1106_Init(); // 初始化SH1106
    // Display.RegisterLCD(&oled, &lcd_oled, "OLED_SH1106"); // 注册LCD显示设备
    // Display.Select("OLED_SH1106"); // 选择当前显示设备
    // Display.Printf("Hello, World!\n");
    // SH1106_Printf(0, 0, SH1106_8X16, "Hello, World!");
    // SH1106_Update(); // 更新显示内容
    LCD_SetFont(&lcd_oled, &JetBrains_Mono_Font_8x16);
    LCD_Printf(&lcd_oled, "This is a test of the LCD terminal mode.\n");
    LCD_Update(&lcd_oled); // 更新显示内容
    while (1)
    {

        // Task_Switch_Tick_Handler(&Shell_Sysfpoint);
        // led.toggle(arg_null);
        // delay.ms(500);
    }
    return 0;
}
