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
    // mpu_dmp_init();                     // 初始化MPU6050 DMP功能


    // if (HMC5883L_Init() != 0)
    // {
    //     printf("HMC5883L初始化失败！请检查连接。\r\n");
    // }
    // printf("HMC5883L初始化成功！\r\n");

    // /* 2. 读取设备ID确认通信正常 */
    // printf("设备ID: 0x%02X (应为0x48)\r\n", HMC_GetID());
    while (1)
    {
        // mpu_dmp_get_data(&pitch, &roll, &yaw);
        // //printf("Pitch: %.2f, Roll: %.2f, Yaw: %.2f\r\n", pitch, roll, yaw);
        // if (HMC5883L_IsDataReady())
        // {
        //     /* 读取原始数据 */
        //     printf("%.2f, %.2f, %.2f, %.2f\r\n", HMC5883L_GetHeading(), pitch, roll, yaw);
        // }
        // delay(100); // 100ms读取一次
    }
    return 0;
}
