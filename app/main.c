#include "main.h"
#include <stdint.h>
#include <sh1106/sh1106.h>
#include <display/df_display.h>
#include <lcd/df_lcd.h>
#include <lcd/df_fonts.h>
#include <mpu6050/inv_mpu.h>
#include <config.h>
float pitch, roll, yaw;

int main()
{
    Device_Registration(Dev_info_poor); // 初始化设备模型
    // mpu_dmp_init();                     // 初始化MPU6050 DMP功能
    while (1)
    {
        // SH1106_CheakDevice();                  // 检测SH1106设备
        // mpu_dmp_get_data(&pitch, &roll, &yaw); // 获取DMP数据
        printf("%.2f\n", pitch);
        pitch += 0.01f;
        // led.toggle(arg_null);
        // delay.ms(500);
    }
    return 0;
}
