#include "main.h"
#include <stdint.h>
#include <sh1106/sh1106.h>
#include <display/df_display.h>
#include <lcd/df_lcd.h>
#include <lcd/df_fonts.h>


int main()
{
    Device_Registration(Dev_info_poor); // 初始化设备模型
    LCD_Printf(&lcd_oled, "This is a test of the LCD terminal mode.\n");
    LCD_Update(&lcd_oled); // 更新显示内容
    while (1)
    {

        // Task_Switch_Tick_Handler(&Shell_Sysfpoint);
        led.toggle(arg_null);
        delay.ms(500);
    }
    return 0;
}
