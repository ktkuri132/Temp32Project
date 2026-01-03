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
    while (1)
    {
        LCD_Printf(&lcd_sh1106, "Hello, World!\n");
        log_flush();
    }
    return 0;
}
