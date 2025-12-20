#ifndef __SH1106_H_
#define __SH1106_H_

#include <config.h>
#ifdef SH1106
#include "fonts.h"
#include <stdarg.h>
#include <string.h>
// #define Peripheral_SPI      // 此处定义外设自带SPI
#define Peripheral_IIC // 此处定义外设自带IIC


#define SH1106_WIDTH 128
#define SH1106_HEIGHT 64

/* 定义1.3寸SH1106地址及其寄存器  */
// 7位SH1106地址  stm32上SH1106的IIC地址为0x78

#define SH1106_Data_Mode 0x40
#define SH1106_Command_Mode 0x00

#ifdef On_Chip_IIC

#undef Peripheral_SPI
#undef Peripheral_IIC
/* 定义IIC端口  */
// #define SH1106_I2C_PORT i2c1

#ifdef __HARDI2C_
#define SH1106_ADDRESS 0x78
/* 江科大SH1106 IIC操作接口   */
#define SH1106_WriteCommand(Command) Hard_IIC_Send_Byte(SH1106_ADDRESS, SH1106_Command_Mode, Command)
#define SH1106_WriteData(Data, Count) Hard_IIC_Wirter_Data(SH1106_ADDRESS, SH1106_Data_Mode, Data, Count)
#define SH1106_GPIO_Init() Hard_IIC_Init()

#elif defined __SOFTI2C_
#define SH1106_ADDRESS 0x78
#define SH1106_WriteCommand(Command) Soft_IIC_Write_Byte(&i2c_Dev, SH1106_ADDRESS, SH1106_Command_Mode, Command)  // Soft_IIC_WriteByte(SH1106_ADDRESS,SH1106_Command_Mode,Command)
#define SH1106_WriteData(Data, Count) Soft_IIC_Write_Len(&i2c_Dev, SH1106_ADDRESS, SH1106_Data_Mode, Count, Data) // Soft_IIC_WriteData(SH1106_ADDRESS,SH1106_Data_Mode,Data,Count)
#define SH1106_GPIO_Init() Soft_IIC_Init(&i2c_Dev)                                                                // Soft_IIC_Init()

#endif

#elif defined On_Chip_SPI
#undef Peripheral_SPI
#undef Peripheral_IIC

#define SH1106_W_D0(BitValue) spi_Dev.Soft_SPI_SCK(BitValue)        // 写D0（CLK）高低电平
#define SH1106_W_D1(BitValue) spi_Dev.Soft_SPI_MOSI(BitValue)       // 写D1（MOSI）高低电平
#define SH1106_W_DC(BitValue) spi_Dev.Soft_SPI_CS2(BitValue)        // 写DC（数据/命令选择）高低电平
#define SH1106_W_RES(BitValue) spi_Dev.Soft_SPI_CS3(BitValue)       // 写RES（复位）高低电平
#define SH1106_W_CS(BitValue) spi_Dev.Soft_SPI_CS(BitValue)         // 写CS（片选）高低电平
#define SH1106_SPI_SendByte(Byte) Soft_SPI_SendByte(&spi_Dev, Byte) // SPI发送一个字节
#define SH1106_GPIO_Init(void) \
    {                          \
        /*置引脚默认电平*/     \
        SH1106_W_D0(0);        \
        SH1106_W_D1(1);        \
        SH1106_W_RES(1);       \
        SH1106_W_DC(1);        \
    }

#endif
/*FontSize参数取值*/
/*此参数值不仅用于判断，而且用于计算横向字符偏移，默认值为字体像素宽度*/
#define SH1106_8X16 8
#define SH1106_6X8 6

/*IsFilled参数数值*/
#define SH1106_UNFILLED 0
#define SH1106_FILLED 1

/*初始化函数*/
uint8_t SH1106_Init(void);

/*更新函数*/
void SH1106_Update(void);
void SH1106_UpdateArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);

/*显存控制函数*/
void SH1106_Clear(void);
void SH1106_ClearArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);
void SH1106_Reverse(void);
void SH1106_ReverseArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);

/*显示函数*/
void SH1106_ShowChar(int16_t X, int16_t Y, char Char, uint8_t FontSize);
void SH1106_ShowString(int16_t X, int16_t Y, char *String, uint8_t FontSize);
void SH1106_ShowNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);
void SH1106_ShowSignedNum(int16_t X, int16_t Y, int32_t Number, uint8_t Length, uint8_t FontSize);
void SH1106_ShowHexNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);
void SH1106_ShowBinNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);
void SH1106_ShowFloatNum(int16_t X, int16_t Y, double Number, uint8_t IntLength, uint8_t FraLength, uint8_t FontSize);
void SH1106_ShowImage(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image);
void SH1106_Printf(int16_t X, int16_t Y, uint8_t FontSize, char *format, ...);

/*绘图函数*/
void SH1106_DrawPoint(int16_t X, int16_t Y);
uint32_t SH1106_GetPoint(uint16_t X, uint16_t Y);
void SH1106_DrawLine(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1);
void SH1106_DrawRectangle(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, uint8_t IsFilled);
void SH1106_DrawTriangle(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1, int16_t X2, int16_t Y2, uint8_t IsFilled);
void SH1106_DrawCircle(int16_t X, int16_t Y, uint8_t Radius, uint8_t IsFilled);
void SH1106_DrawEllipse(int16_t X, int16_t Y, uint8_t A, uint8_t B, uint8_t IsFilled);
void SH1106_DrawArc(int16_t X, int16_t Y, uint8_t Radius, int16_t StartAngle, int16_t EndAngle, uint8_t IsFilled);

#endif
#endif