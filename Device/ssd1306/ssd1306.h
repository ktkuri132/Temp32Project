#ifndef __SSD1306_H_
#define __SSD1306_H_

#include <config.h>
#ifdef USE_DEVICE_SSD1306
#include "fonts.h"
#include <stdarg.h>
#include <string.h>
// #define Peripheral_SPI      // 此处定义外设自带SPI
#define Peripheral_IIC // 此处定义外设自带IIC

#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64

/* 定义1.3寸SSD1306地址及其寄存器  */
// 7位SSD1306地址  stm32上SSD1306的IIC地址为0x78

#define SSD1306_Data_Mode 0x40
#define SSD1306_Command_Mode 0x00

#ifdef On_Chip_IIC

#undef Peripheral_SPI
#undef Peripheral_IIC
/* 定义IIC端口  */
// #define SSD1306_I2C_PORT i2c1

#ifdef __HARDI2C_
#define SSD1306_ADDRESS 0x78
/* 江科大SSD1306 IIC操作接口   */
#define SSD1306_WriteCommand(Command) Hard_IIC_Send_Byte(SSD1306_ADDRESS, SSD1306_Command_Mode, Command)
#define SSD1306_WriteData(Data, Count) Hard_IIC_Wirter_Data(SSD1306_ADDRESS, SSD1306_Data_Mode, Data, Count)
#define SSD1306_GPIO_Init() Hard_IIC_Init()

#elif defined __SOFTI2C_
#define SSD1306_ADDRESS 0x78
#define SSD1306_WriteCommand(Command) Soft_IIC_Write_Byte(&i2c_Dev, SSD1306_ADDRESS, SSD1306_Command_Mode, Command)  // Soft_IIC_WriteByte(SSD1306_ADDRESS,SSD1306_Command_Mode,Command)
#define SSD1306_WriteData(Data, Count) Soft_IIC_Write_Len(&i2c_Dev, SSD1306_ADDRESS, SSD1306_Data_Mode, Count, Data) // Soft_IIC_WriteData(SSD1306_ADDRESS,SSD1306_Data_Mode,Data,Count)
#define SSD1306_GPIO_Init() Soft_IIC_Init(&i2c_Dev)                                                                  // Soft_IIC_Init()

#endif

#elif defined On_Chip_SPI
#undef Peripheral_SPI
#undef Peripheral_IIC

#define SSD1306_W_D0(BitValue) spi_Dev.Soft_SPI_SCK(BitValue)        // 写D0（CLK）高低电平
#define SSD1306_W_D1(BitValue) spi_Dev.Soft_SPI_MOSI(BitValue)       // 写D1（MOSI）高低电平
#define SSD1306_W_DC(BitValue) spi_Dev.Soft_SPI_CS2(BitValue)        // 写DC（数据/命令选择）高低电平
#define SSD1306_W_RES(BitValue) spi_Dev.Soft_SPI_CS3(BitValue)       // 写RES（复位）高低电平
#define SSD1306_W_CS(BitValue) spi_Dev.Soft_SPI_CS(BitValue)         // 写CS（片选）高低电平
#define SSD1306_SPI_SendByte(Byte) Soft_SPI_SendByte(&spi_Dev, Byte) // SPI发送一个字节
#define SSD1306_GPIO_Init(void) \
    {                           \
        /*置引脚默认电平*/      \
        SSD1306_W_D0(0);        \
        SSD1306_W_D1(1);        \
        SSD1306_W_RES(1);       \
        SSD1306_W_DC(1);        \
    }

#endif
/*FontSize参数取值*/
/*此参数值不仅用于判断，而且用于计算横向字符偏移，默认值为字体像素宽度*/
#define SSD1306_8X16 8
#define SSD1306_6X8 6

/*IsFilled参数数值*/
#define SSD1306_UNFILLED 0
#define SSD1306_FILLED 1

/*初始化函数*/
void SSD1306_Init(void);

/*更新函数*/
void SSD1306_Update(void);
void SSD1306_UpdateArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);

/*显存控制函数*/
void SSD1306_Clear(void);
void SSD1306_ClearArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);
void SSD1306_Reverse(void);
void SSD1306_ReverseArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);

/*显示函数*/
void SSD1306_ShowChar(int16_t X, int16_t Y, char Char, uint8_t FontSize);
void SSD1306_ShowString(int16_t X, int16_t Y, char *String, uint8_t FontSize);
void SSD1306_ShowNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);
void SSD1306_ShowSignedNum(int16_t X, int16_t Y, int32_t Number, uint8_t Length, uint8_t FontSize);
void SSD1306_ShowHexNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);
void SSD1306_ShowBinNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);
void SSD1306_ShowFloatNum(int16_t X, int16_t Y, double Number, uint8_t IntLength, uint8_t FraLength, uint8_t FontSize);
void SSD1306_ShowImage(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image);
void SSD1306_Printf(int16_t X, int16_t Y, uint8_t FontSize, char *format, ...);

/*绘图函数*/
void SSD1306_DrawPoint(int16_t X, int16_t Y);
uint8_t SSD1306_GetPoint(int16_t X, int16_t Y);
void SSD1306_DrawLine(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1);
void SSD1306_DrawRectangle(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, uint8_t IsFilled);
void SSD1306_DrawTriangle(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1, int16_t X2, int16_t Y2, uint8_t IsFilled);
void SSD1306_DrawCircle(int16_t X, int16_t Y, uint8_t Radius, uint8_t IsFilled);
void SSD1306_DrawEllipse(int16_t X, int16_t Y, uint8_t A, uint8_t B, uint8_t IsFilled);
void SSD1306_DrawArc(int16_t X, int16_t Y, uint8_t Radius, int16_t StartAngle, int16_t EndAngle, uint8_t IsFilled);

#endif
#endif