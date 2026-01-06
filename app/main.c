#include "main.h"
#include <stdint.h>
#include <sh1106/sh1106.h>
#include <display/df_display.h>
#include <lcd/df_lcd.h>
#include <lcd/df_fonts.h>
#include <mpu6050/inv_mpu.h>
#include <hmc588/hmc588.h>
#include <config.h>

int main()
{
    led.on(arg_null);
    df_dev_t mpu6050;
    if (df_dev_find(Dev_info_poor, MPU6050_NAME, &mpu6050))
    {
        error("main", "MPU6050 device not found!\n");
        while (1);
    }
    while (1)
    {
        mpu6050.read(arg_ptr(&mpu6050));

    }
    return 0;
}
