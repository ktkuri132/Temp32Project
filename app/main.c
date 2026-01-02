#include "main.h"
#include <stdint.h>
#include <sh1106/sh1106.h>
#include <display/df_display.h>
#include <lcd/df_lcd.h>
#include <lcd/df_fonts.h>
#include <mpu6050/inv_mpu.h>
#include <hmc588/hmc588.h>
#include <config.h>
float pitch, roll, yaw;
int16_t mag_x, mag_y, mag_z;
int main()
{
    Device_Registration(Dev_info_poor); // 初始化设备模型

    while (1)
    {
        log_flush();
    }
    return 0;
}
