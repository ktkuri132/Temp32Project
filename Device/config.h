#ifndef __CONFIG_H_
#define __CONFIG_H_

#include <stdint.h>

// 选择使用的外围设备
 #define SH1106
//#define SSD1306
#define MPU6050

// 通信总线选择
#define __SOFTI2C_


#ifdef __SOFTI2C_
#include <i2c/df_iic.h>
extern SIAS i2c1_bus;
#define i2c_Dev i2c1_bus
#endif

#ifdef SH1106

#include <lcd/df_lcd.h>
#define On_Chip_IIC
void SH1106_SetPixel(uint16_t x, uint16_t y, uint32_t color);
void SH1106_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color);
#endif

#ifdef SSD1306

#include <lcd/df_lcd.h>
#define On_Chip_IIC

#endif

#ifdef MPU6050
#include <df_delay.h>
#define i2c_dev i2c1_bus
extern Dt delay;
uint8_t mpu6050_i2c_write(uint8_t addr, uint8_t reg, uint16_t length, uint8_t *data);
uint8_t mpu6050_i2c_read(uint8_t addr, uint8_t reg, uint16_t length, uint8_t *data);
#endif


#endif