/**
 * @file f103_spi.h
 * @brief STM32F103系列SPI公共驱动头文件
 */

#ifndef __F103_SPI_H
#define __F103_SPI_H

#include <stm32f10x.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        F103_SPI1 = 0,
        F103_SPI2,
        F103_SPI_MAX
    } f103_spi_port_t;

    typedef enum
    {
        F103_SPI_MODE_MASTER = 0,
        F103_SPI_MODE_SLAVE
    } f103_spi_mode_t;

    typedef enum
    {
        F103_SPI_DIV_2 = 0,
        F103_SPI_DIV_4,
        F103_SPI_DIV_8,
        F103_SPI_DIV_16,
        F103_SPI_DIV_32,
        F103_SPI_DIV_64,
        F103_SPI_DIV_128,
        F103_SPI_DIV_256
    } f103_spi_prescaler_t;

    typedef struct
    {
        f103_spi_port_t port;
        f103_spi_mode_t mode;
        f103_spi_prescaler_t prescaler;
        uint8_t cpol;      /* 0: 低电平空闲, 1: 高电平空闲 */
        uint8_t cpha;      /* 0: 第一边沿采样, 1: 第二边沿采样 */
        uint8_t data_size; /* 8 或 16 */
    } f103_spi_config_t;

    int f103_spi_init(const f103_spi_config_t *config);
    int f103_spi_init_quick(f103_spi_port_t port, f103_spi_prescaler_t prescaler);
    uint8_t f103_spi_transfer(f103_spi_port_t port, uint8_t data);
    void f103_spi_send(f103_spi_port_t port, const uint8_t *data, uint32_t len);
    void f103_spi_recv(f103_spi_port_t port, uint8_t *data, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* __F103_SPI_H */
