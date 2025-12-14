#ifndef __DF_SPI_H_
#define __DF_SPI_H_

#ifndef On_Chip_SPI
#define On_Chip_SPI

#ifndef __SOFTSPI_
#define __SOFTSPI_

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define CPOL 0  // 时钟极性
#define CPHA 0  // 时钟相位

#define MODE1 1  // SPI工作模式1
#define MODE2 2  // SPI工作模式2
#define MODE3 3  // SPI工作模式3
#define MODE4 4  // SPI工作模式4

/*定义SPI需要使用的片选引脚*/
#define SPI_CS 1
#define SPI_CS2 1
#define SPI_CS3 1

/*定义SPI使用的方向*/
#define SPI_MOSI 1
#define SPI_MISO 0

#if (CPOL == 0 && CPHA == 0)
#define SPI_MODE MODE1  // CPOL=0,CPHA=0
#elif (CPOL == 0 && CPHA == 1)
#define SPI_MODE MODE2  // CPOL=0,CPHA=1
#elif (CPOL == 1 && CPHA == 0)
#define SPI_MODE MODE3  // CPOL=1,CPHA=0
#elif (CPOL == 1 && CPHA == 1)
#define SPI_MODE MODE4  // CPOL=1,CPHA=1
#endif

#endif

typedef struct Soft_SPI_APIs_Struct {
    void (*Soft_SPI_GPIO_Port_Init)(void);  // SPI GPIO端口初始化函数
    void (*Soft_SPI_SCK)(uint8_t state);    // SCK线控制函数
    void (*Soft_SPI_MOSI)(uint8_t state);   // MOSI线控制函数
    uint8_t (*Soft_SPI_MISO)(void);         // 读取MISO线状态
    void (*Soft_SPI_CS)(uint8_t state);     // CS线控制函数
    void (*Soft_SPI_CS2)(uint8_t state);    // CS2线控制函数
    void (*Soft_SPI_CS3)(uint8_t state);    // CS3线控制函数
} SSAS;


#if (MISO && MOSI) || (MISO && !MOSI)
uint8_t* Soft_SPI_SwapData(SSAS* spi_Dev, uint8_t* data, uint16_t len);  // SPI数据交换函数
uint8_t Soft_SPI_SwapBytes(SSAS *spi_Dev, uint8_t data);
uint8_t Soft_SPI_RecvByte(SSAS *spi_Dev,uint8_t Byte);
#else if (!MISO && MOSI)
void Soft_SPI_SendByte(SSAS *spi_Dev,uint8_t Byte);
void Soft_SPI_SwapBytes(SSAS *spi_Dev, uint8_t data);
void Soft_SPI_SwapData(SSAS *spi_Dev, uint8_t *data, uint16_t len);
#endif

#endif

#endif