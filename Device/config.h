#ifndef __CONFIG_H_
#define __CONFIG_H_

#include <stdint.h>

// 选择使用的外围设备
#define SH1106
#include <i2c/df_iic.h>
#include <lcd/df_lcd.h>
#define On_Chip_IIC
#define __SOFTI2C_

extern SIAS i2c1_bus;
#define i2c_Dev i2c1_bus


#endif