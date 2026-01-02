#ifndef __SH1106_H_
#define __SH1106_H_

#include <config.h>
#ifdef USE_DEVICE_SH1106
#include <stdarg.h>
#include <string.h>
#include <device_hal.h>
#define SH1106_WIDTH 128
#define SH1106_HEIGHT 64

/* SH1106 I2C地址和寄存器模式 */
#define SH1106_ADDRESS 0x78
#define SH1106_Data_Mode 0x40
#define SH1106_Command_Mode 0x00

/*============================ HAL接口声明 ============================*/
/**
 * @brief SH1106 I2C HAL接口实例
 */
extern device_i2c_hal_t *sh1106_i2c_hal;

/**
 * @brief SH1106 SPI HAL接口实例
 */
extern device_spi_hal_t *sh1106_spi_hal;

/**
 * @brief 初始化SH1106并绑定I2C HAL接口
 * @param hal I2C HAL接口指针
 * @return 0-成功，非0-失败
 */
int SH1106_Init_HAL_I2C(device_i2c_hal_t *hal);

/**
 * @brief 初始化SH1106并绑定SPI HAL接口
 * @param hal SPI HAL接口指针
 * @return 0-成功，非0-失败
 */
int SH1106_Init_HAL_SPI(device_spi_hal_t *hal);

/**
 * @brief 初始化SH1106并绑定HAL接口（兼容旧接口，默认使用I2C）
 * @param hal I2C HAL接口指针
 * @return 0-成功，非0-失败
 */
int SH1106_Init_HAL(device_i2c_hal_t *hal);
/*FontSize参数取值*/
/*此参数值不仅用于判断，而且用于计算横向字符偏移，默认值为字体像素宽度*/
#define SH1106_8X16 8
#define SH1106_6X8 6

/*IsFilled参数数值*/
#define SH1106_UNFILLED 0
#define SH1106_FILLED 1

/*初始化函数*/
uint8_t SH1106_Init(void);
uint8_t SH1106_CheakDevice(void);
/*更新函数*/
void SH1106_Update(void);
void SH1106_UpdateArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);

/*显存控制函数*/
void SH1106_Clear(void);
void SH1106_ClearArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);
void SH1106_Reverse(void);
void SH1106_ReverseArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);

/*绘图函数*/
void SH1106_DrawPoint(int16_t X, int16_t Y);
uint32_t SH1106_GetPoint(uint16_t X, uint16_t Y);
void SH1106_SetPixel(uint16_t x, uint16_t y, uint32_t color);
void SH1106_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color);

#endif
#endif