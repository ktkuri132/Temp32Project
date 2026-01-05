/**
 * @file spi_bus.c
 * @brief STM32F1 软件SPI总线驱动
 * @note 使用 f103_gpio 接口实现软件SPI
 */

#include <spi/df_spi.h>
#include "f103_gpio.h"
#include "config.h"
#include "driver.h"

/*===========================================================================*/
/*                         SPI1 引脚配置                                      */
/*===========================================================================*/

#define SPI1_SCK_PORT F103_GPIOA
#define SPI1_SCK_PIN F103_PIN_5
// #define SPI1_MISO_PORT F103_GPIOA
// #define SPI1_MISO_PIN F103_PIN_6
#define SPI1_MOSI_PORT F103_GPIOA
#define SPI1_MOSI_PIN F103_PIN_7
#define SPI1_NSS_PORT F103_GPIOA
#define SPI1_NSS_PIN F103_PIN_4

/*===========================================================================*/
/*                         SPI1 GPIO 操作函数                                 */
/*===========================================================================*/

/**
 * @brief SPI1引脚初始化
 */
void spi1_gpio_init(void)
{
    /* 配置 SCK, MOSI, NSS 为推挽输出50MHz */
    f103_gpio_init_quick(SPI1_SCK_PORT, SPI1_SCK_PIN,
                         F103_GPIO_MODE_OUT_PP, F103_GPIO_SPEED_50MHZ);
    f103_gpio_init_quick(SPI1_MOSI_PORT, SPI1_MOSI_PIN,
                         F103_GPIO_MODE_OUT_PP, F103_GPIO_SPEED_50MHZ);
    f103_gpio_init_quick(SPI1_NSS_PORT, SPI1_NSS_PIN,
                         F103_GPIO_MODE_OUT_PP, F103_GPIO_SPEED_50MHZ);

    /* 默认状态：SCK低电平，NSS高电平（未选中）*/
    f103_gpio_reset(SPI1_SCK_PORT, SPI1_SCK_PIN);
    f103_gpio_set(SPI1_NSS_PORT, SPI1_NSS_PIN);
}

/**
 * @brief SPI1 SCK线控制
 */
void spi1_sck(uint8_t state)
{
    if (state)
    {
        f103_gpio_set(SPI1_SCK_PORT, SPI1_SCK_PIN);
    }
    else
    {
        f103_gpio_reset(SPI1_SCK_PORT, SPI1_SCK_PIN);
    }
}

/**
 * @brief SPI1 MOSI线控制
 */
void spi1_mosi(uint8_t state)
{
    if (state)
    {
        f103_gpio_set(SPI1_MOSI_PORT, SPI1_MOSI_PIN);
    }
    else
    {
        f103_gpio_reset(SPI1_MOSI_PORT, SPI1_MOSI_PIN);
    }
}

/**
 * @brief SPI1 CS线控制
 */
void spi1_cs(uint8_t state)
{
    if (state)
    {
        f103_gpio_set(SPI1_NSS_PORT, SPI1_NSS_PIN);
    }
    else
    {
        f103_gpio_reset(SPI1_NSS_PORT, SPI1_NSS_PIN);
    }
}

/**
 * @brief SPI1软件SPI底层接口实例
 */
df_soft_spi_t spi1_soft = {
    .gpio_init = spi1_gpio_init,
    .sck = spi1_sck,
    .mosi = spi1_mosi,
    .miso = NULL, /* 未使用MISO */
    .cs = spi1_cs,
    .cs2 = NULL,
    .cs3 = NULL};

/*===========================================================================*/
/*                         SPI1 统一接口实现                                  */
/*===========================================================================*/

/**
 * @brief SPI1 初始化
 * @param arg 传参 arg_null
 * @return 0成功，其他失败
 */
int spi1_init(df_arg_t arg)
{
    Soft_SPI_Init(&spi1_soft);
    spi1_bus.init_flag = true;
    return 0;
}

/**
 * @brief SPI1 去初始化
 * @param arg 传参 arg_null
 * @return 0成功
 */
int spi1_deinit(df_arg_t arg)
{
    spi1_bus.init_flag = false;
    return 0;
}

/**
 * @brief SPI1 发送数据
 * @param arg 传参 arg_ptr(df_spi_xfer_t*)
 * @return 0成功，其他失败
 */
int spi1_send(df_arg_t arg)
{
    df_spi_xfer_t *xfer = (df_spi_xfer_t *)arg.ptr;
    if (xfer == NULL || xfer->tx_buf == NULL)
    {
        return -1;
    }

    Soft_SPI_SwapData(spi1_bus.soft_spi, xfer->tx_buf, xfer->len);

    return 0;
}

/**
 * @brief SPI1 片选控制
 * @param arg 传参 arg_s32(0/1) 0=选中(低电平)，1=释放(高电平)
 * @return 0成功
 */
int spi1_cs_ctrl(df_arg_t arg)
{
    spi1_cs(arg.s32 ? 1 : 0);
    return 0;
}

/*===========================================================================*/
/*                         SPI1 底层接口实例                                  */
/*===========================================================================*/



/**
 * @brief SPI1总线实例（统一接口）
 */
df_spi_t spi1_bus = {
    .init_flag = false,
    .num = 1,
    .name = "SPI1_SOFT",
    .init = NULL,
    .deinit = spi1_deinit,
    .transfer = NULL, /* 仅发送模式，不实现 */
    .send = NULL,
    .receive = NULL, /* 仅发送模式，不实现 */
    .cs_ctrl = NULL,
    .soft_spi = &spi1_soft};