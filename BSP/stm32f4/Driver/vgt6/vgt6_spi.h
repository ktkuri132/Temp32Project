/**
 * @file vgt6_spi.h
 * @brief STM32F407VGT6 SPI驱动 - 复用VET6驱动
 */

#ifndef __VGT6_SPI_H
#define __VGT6_SPI_H

#include "../vet6/vet6_spi.h"

/* 类型别名 */
typedef vet6_spi_t vgt6_spi_t;
typedef vet6_spi_prescaler_t vgt6_spi_prescaler_t;
typedef vet6_spi_bits_t vgt6_spi_bits_t;
typedef vet6_spi_config_t vgt6_spi_config_t;
typedef vet6_spi_handle_t vgt6_spi_handle_t;

/* 枚举别名 */
#define VGT6_SPI1 VET6_SPI1
#define VGT6_SPI2 VET6_SPI2
#define VGT6_SPI3 VET6_SPI3
#define VGT6_SPI_MAX VET6_SPI_MAX

#define VGT6_SPI_BITS_8 VET6_SPI_BITS_8
#define VGT6_SPI_BITS_16 VET6_SPI_BITS_16

/* 函数别名 */
#define vgt6_spi_clk_enable vet6_spi_clk_enable
#define vgt6_spi_clk_disable vet6_spi_clk_disable
#define vgt6_spi_init vet6_spi_init
#define vgt6_spi_init_master vet6_spi_init_master
#define vgt6_spi_deinit vet6_spi_deinit
#define vgt6_spi_enable vet6_spi_enable
#define vgt6_spi_disable vet6_spi_disable
#define vgt6_spi_transfer_byte vet6_spi_transfer_byte
#define vgt6_spi_send vet6_spi_send
#define vgt6_spi_receive vet6_spi_receive
#define vgt6_spi_transfer vet6_spi_transfer

#endif /* __VGT6_SPI_H */
