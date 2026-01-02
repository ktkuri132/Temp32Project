/**
 * @file f103_spi.c
 * @brief STM32F103 系列 SPI 驱动实现
 */

#include "f103_spi.h"
#include "f103_gpio.h"

static SPI_TypeDef *const spi_base_table[F103_SPI_MAX] = {
    SPI1, SPI2};

static void f103_spi_clk_enable(f103_spi_port_t port)
{
    switch (port)
    {
    case F103_SPI1:
        RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
        break;
    case F103_SPI2:
        RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
        break;
    default:
        break;
    }
}

static void f103_spi_gpio_init(f103_spi_port_t port)
{
    switch (port)
    {
    case F103_SPI1:
        /* PA5: SCK, PA6: MISO, PA7: MOSI */
        f103_gpio_init_quick(F103_GPIOA, F103_PIN_5, F103_GPIO_MODE_AF_PP, F103_GPIO_SPEED_50MHZ);
        f103_gpio_init_quick(F103_GPIOA, F103_PIN_6, F103_GPIO_MODE_IN_FLOATING, F103_GPIO_SPEED_50MHZ);
        f103_gpio_init_quick(F103_GPIOA, F103_PIN_7, F103_GPIO_MODE_AF_PP, F103_GPIO_SPEED_50MHZ);
        break;
    case F103_SPI2:
        /* PB13: SCK, PB14: MISO, PB15: MOSI */
        f103_gpio_init_quick(F103_GPIOB, F103_PIN_13, F103_GPIO_MODE_AF_PP, F103_GPIO_SPEED_50MHZ);
        f103_gpio_init_quick(F103_GPIOB, F103_PIN_14, F103_GPIO_MODE_IN_FLOATING, F103_GPIO_SPEED_50MHZ);
        f103_gpio_init_quick(F103_GPIOB, F103_PIN_15, F103_GPIO_MODE_AF_PP, F103_GPIO_SPEED_50MHZ);
        break;
    default:
        break;
    }
}

int f103_spi_init(const f103_spi_config_t *config)
{
    if (config == NULL || config->port >= F103_SPI_MAX)
        return -1;

    SPI_TypeDef *spi = spi_base_table[config->port];

    f103_spi_clk_enable(config->port);
    f103_spi_gpio_init(config->port);

    /* 禁用SPI */
    spi->CR1 = 0;

    uint32_t cr1 = 0;

    /* 主从模式 */
    if (config->mode == F103_SPI_MODE_MASTER)
    {
        cr1 |= SPI_CR1_MSTR | SPI_CR1_SSI | SPI_CR1_SSM;
    }

    /* 波特率分频 */
    cr1 |= (config->prescaler << 3);

    /* 时钟极性和相位 */
    if (config->cpol)
        cr1 |= SPI_CR1_CPOL;
    if (config->cpha)
        cr1 |= SPI_CR1_CPHA;

    /* 数据位宽 */
    if (config->data_size == 16)
        cr1 |= SPI_CR1_DFF;

    spi->CR1 = cr1;

    /* 使能SPI */
    spi->CR1 |= SPI_CR1_SPE;

    return 0;
}

int f103_spi_init_quick(f103_spi_port_t port, f103_spi_prescaler_t prescaler)
{
    f103_spi_config_t config = {
        .port = port,
        .mode = F103_SPI_MODE_MASTER,
        .prescaler = prescaler,
        .cpol = 0,
        .cpha = 0,
        .data_size = 8};
    return f103_spi_init(&config);
}

uint8_t f103_spi_transfer(f103_spi_port_t port, uint8_t data)
{
    if (port >= F103_SPI_MAX)
        return 0;

    SPI_TypeDef *spi = spi_base_table[port];

    while (!(spi->SR & SPI_SR_TXE))
        ;
    spi->DR = data;

    while (!(spi->SR & SPI_SR_RXNE))
        ;
    return (uint8_t)spi->DR;
}

void f103_spi_send(f103_spi_port_t port, const uint8_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
    {
        f103_spi_transfer(port, data[i]);
    }
}

void f103_spi_recv(f103_spi_port_t port, uint8_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
    {
        data[i] = f103_spi_transfer(port, 0xFF);
    }
}
