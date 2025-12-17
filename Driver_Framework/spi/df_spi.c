#include <spi/df_spi.h>
#include <stdint.h>

void Soft_SPI_Init(SSAS *spi_dev) {
    if (spi_dev->Soft_SPI_GPIO_Port_Init != NULL) {
        spi_dev->Soft_SPI_GPIO_Port_Init();  // Initialize SPI GPIO ports
    }
    spi_dev->Soft_SPI_SCK(1);   // Set SCK high
    spi_dev->Soft_SPI_MOSI(1);  // Set MOSI high
    spi_dev->Soft_SPI_CS(1);    // Set CS high
#if (SPI_CS2)
    spi_dev->Soft_SPI_CS2(1);  // Set CS2 high
#endif
#if (SPI_CS3)
    spi_dev->Soft_SPI_CS3(1);  // Set CS3 high
#endif
}

#if (!SPI_MISO && SPI_MOSI)
void Soft_SPI_SendByte(SSAS *spi_Dev,uint8_t Byte) {
    uint8_t i;

    /*循环8次，主机依次发送数据的每一位*/
    for (i = 0; i < 8; i++) {
        /*使用掩码的方式取出Byte的指定一位数据并写入到D1线*/
        /*两个!的作用是，让所有非零的值变为1*/
        spi_Dev->Soft_SPI_MOSI(!!(Byte & (0x80 >> i)));
        spi_Dev->Soft_SPI_SCK(1); // 拉高D0，从机在D0上升沿读取SDA
        spi_Dev->Soft_SPI_SCK(0); // 拉低D0，主机开始发送下一位数据
    }
}

#else if (SPI_MISO && SPI_MOSI) || (SPI_MISO && !SPI_MOSI)

uint8_t Soft_SPI_RecvByte(SSAS *spi_Dev,uint8_t Byte) {
    uint8_t i;
        uint8_t DATA_IN = 0;

        /*循环8次，主机依次接收数据的每一位*/
        for (i = 0; i < 8; i++) {
                spi_Dev->Soft_SPI_SCK(1); // 拉高D0，从机在D0上升沿读取SDA
                if (spi_Dev->Soft_SPI_MISO()) {
                DATA_IN |= (1 << (7 - i)); // 读取MISO线状态
                }
                spi_Dev->Soft_SPI_SCK(0); // 拉低D0，主机开始接收下一位数据
        }
        return DATA_IN;
}

#endif

#if (SPI_MISO && SPI_MOSI) || (SPI_MISO && !SPI_MOSI)
uint8_t *Soft_SPI_SwapData(SSAS *spi_Dev, uint8_t *data, uint16_t len) {
#else if (!SPI_MISO && SPI_MOSI)
void Soft_SPI_SwapData(SSAS *spi_Dev, uint8_t *data, uint16_t len) {
#endif
    uint8_t i, j;
    uint8_t *DATA_OUT = data;
    uint8_t *DATA_IN;
    for (i = 0; i < len; i++) {
        for (j = 0; j < 8; j++) {
#if (SPI_MOSI)
            // 数据输出
            spi_Dev->Soft_SPI_MOSI(!!(DATA_OUT[i] & (0x80 >> i)));
#endif
#if (SPI_MISO)
            // 数据输入
            if (spi_Dev->Soft_SPI_MISO()) {
                DATA_IN[i] |= (1 << (7 - j));
            } else {
                DATA_IN[i] &= ~(1 << (7 - j));
            }
#endif
            spi_Dev->Soft_SPI_SCK(1);
            spi_Dev->Soft_SPI_SCK(0);
        }
    }

    #if (MISO && MOSI) || (MISO && !MOSI)
return DATA_IN;
#else if (!MISO && MOSI)

#endif
}

#if (SPI_MISO && SPI_MOSI)
uint8_t Soft_SPI_SwapBytes(SSAS *spi_Dev, uint8_t data) {
#else if (!SPI_MISO && SPI_MOSI)
void Soft_SPI_SwapBytes(SSAS *spi_Dev, uint8_t data) {
#endif
    uint8_t i;
    uint8_t DATA_IN = 0;
    /*循环8次，主机依次发送数据的每一位*/
    for (i = 0; i < 8; i++) {
#if (SPI_MOSI)
        spi_Dev->Soft_SPI_MOSI(!!(data & (0x80 >> i)));
#endif
#if (SPI_MISO)
        // 数据输入
        if (spi_Dev->Soft_SPI_MISO()) {
            DATA_IN |= (1 << (7 - i));
        } else {
            DATA_IN &= ~(1 << (7 - i));
        }
#endif
        spi_Dev->Soft_SPI_SCK(1); // 拉高D0，从机在D0上升沿读取SDA
        spi_Dev->Soft_SPI_SCK(0); // 拉低D0，主机开始发送下一位数据
    }
#if (MISO && MOSI) || (MISO && !MOSI)
    return DATA_IN;
#else if (!MISO && MOSI)

#endif
}


