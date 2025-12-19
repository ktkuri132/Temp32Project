#include "main.h"
#include <stdint.h>
#include <sh1106/sh1106.h>
#include <display/df_display.h>
#include <lcd/df_lcd.h>
#include <lcd/df_fonts.h>
#include <mpu6050/inv_mpu.h>

float pitch, roll, yaw;

int main()
{
    Device_Registration(Dev_info_poor); // 初始化设备模型
    // SH1106_Clear();                     // 清屏
    // SH1106_Update();
    mpu_dmp_init();                     // 初始化MPU6050 DMP功能
    
    // SH1106_Update(); // 更新显示

    while (1)
    {
        mpu_dmp_get_data(&pitch, &roll, &yaw); // 获取DMP数据
        // SH1106_Printf(0, 0, SH1106_8X16, "P:%.2f", pitch);
        // SH1106_Update();
        // printf("Pitch: %.2f, Roll: %.2f, Yaw: %.2f\n", pitch, roll, yaw);
        // debug.send(arg_ptr("Hello, World!\r\n"));
        // delay.ms(1000);
        // Task_Switch_Tick_Handler(&Shell_Sysfpoint);
        // LCD_Printf(&lcd_sh1106, "Pitch: %.2f\n", pitch);
        // LCD_Update(&lcd_sh1106);
        // led.toggle(arg_null);
        // delay.ms(500);
    }
    return 0;
}
