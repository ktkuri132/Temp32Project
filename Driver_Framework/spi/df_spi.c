#include <spi/df_spi.h>
#include <stdint.h>

void Soft_SPI_Init(df_soft_spi_t *spi_dev)
{
    if (spi_dev->gpio_init != NULL)
    {
        spi_dev->gpio_init(); // Initialize SPI GPIO ports
    }
    spi_dev->sck(1);  // Set SCK high
    spi_dev->mosi(1); // Set MOSI high
    spi_dev->cs(1);   // Set CS high
#if (SPI_CS2)
    spi_dev->cs2(1); // Set CS2 high
#endif
#if (SPI_CS3)
    spi_dev->cs3(1); // Set CS3 high
#endif
}

#if (!SPI_MISO && SPI_MOSI)
void Soft_SPI_SendByte(df_soft_spi_t *spi_Dev, uint8_t Byte)
{
    uint8_t i;

    /*循环8次，主机依次发送数据的每一位*/
    for (i = 0; i < 8; i++)
    {
        /*使用掩码的方式取出Byte的指定一位数据并写入到D1线*/
        /*两个!的作用是，让所有非零的值变为1*/
        spi_Dev->mosi(!!(Byte & (0x80 >> i)));
        spi_Dev->sck(1); // 拉高D0，从机在D0上升沿读取SDA
        spi_Dev->sck(0); // 拉低D0，主机开始发送下一位数据
    }
}

#else if (SPI_MISO && SPI_MOSI) || (SPI_MISO && !SPI_MOSI)

uint8_t Soft_SPI_RecvByte(df_soft_spi_t *spi_Dev, uint8_t Byte)
{
    uint8_t i;
    uint8_t DATA_IN = 0;

    /*循环8次，主机依次接收数据的每一位*/
    for (i = 0; i < 8; i++)
    {
        spi_Dev->sck(1); // 拉高D0，从机在D0上升沿读取SDA
        if (spi_Dev->miso())
        {
            DATA_IN |= (1 << (7 - i)); // 读取MISO线状态
        }
        spi_Dev->sck(0); // 拉低D0，主机开始接收下一位数据
    }
    return DATA_IN;
}

#endif

#if (SPI_MISO && SPI_MOSI) || (SPI_MISO && !SPI_MOSI)
uint8_t *Soft_SPI_SwapData(df_soft_spi_t *spi_Dev, uint8_t *data, uint16_t len)
{
#else if (!SPI_MISO && SPI_MOSI)
void Soft_SPI_SwapData(df_soft_spi_t *spi_Dev, uint8_t *data, uint16_t len)
{
#endif
    uint8_t i, j;
    uint8_t *DATA_OUT = data;
    uint8_t *DATA_IN;
    for (i = 0; i < len; i++)
    {
        for (j = 0; j < 8; j++)
        {
#if (SPI_MOSI)
            // 数据输出
            spi_Dev->mosi(!!(DATA_OUT[i] & (0x80 >> i)));
#endif
#if (SPI_MISO)
            // 数据输入
            if (spi_Dev->miso())
            {
                DATA_IN[i] |= (1 << (7 - j));
            }
            else
            {
                DATA_IN[i] &= ~(1 << (7 - j));
            }
#endif
            spi_Dev->sck(1);
            spi_Dev->sck(0);
        }
    }

#if (MISO && MOSI) || (MISO && !MOSI)
    return DATA_IN;
#else if (!MISO && MOSI)

#endif
}

#if (SPI_MISO && SPI_MOSI)
uint8_t Soft_SPI_SwapBytes(df_soft_spi_t *spi_Dev, uint8_t data)
{
#else if (!SPI_MISO && SPI_MOSI)
void Soft_SPI_SwapBytes(df_soft_spi_t *spi_Dev, uint8_t data)
{
#endif
    uint8_t i;
    uint8_t DATA_IN = 0;
    /*循环8次，主机依次发送数据的每一位*/
    for (i = 0; i < 8; i++)
    {
#if (SPI_MOSI)
        spi_Dev->mosi(!!(data & (0x80 >> i)));
#endif
#if (SPI_MISO)
        // 数据输入
        if (spi_Dev->miso())
        {
            DATA_IN |= (1 << (7 - i));
        }
        else
        {
            DATA_IN &= ~(1 << (7 - i));
        }
#endif
        spi_Dev->sck(1); // 拉高D0，从机在D0上升沿读取SDA
        spi_Dev->sck(0); // 拉低D0，主机开始发送下一位数据
    }
#if (MISO && MOSI) || (MISO && !MOSI)
    return DATA_IN;
#else if (!MISO && MOSI)

#endif
}
