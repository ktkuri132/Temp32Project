/**
 * @file f407_spi.c
 * @brief STM32F407系列通用SPI板级驱动实现
 * @details 提供SPI1-SPI3的初始化和操作API
 *          适用于所有STM32F407封装型号 (VET6, ZET6, ZGT6等)
 */

#include "f407_spi.h"

/*===========================================================================*/
/*                              内部变量                                      */
/*===========================================================================*/

/* SPI外设基地址表 */
static SPI_TypeDef *const spi_base_table[F407_SPI_MAX] = {
    SPI1, SPI2, SPI3};

/* SPI时钟频率表 */
static const uint32_t spi_clk_table[F407_SPI_MAX] = {
    F407_SPI1_CLK, F407_SPI2_CLK, F407_SPI3_CLK};

/*===========================================================================*/
/*                              时钟控制                                      */
/*===========================================================================*/

/**
 * @brief 使能SPI时钟
 */
void f407_spi_clk_enable(f407_spi_t spi)
{
    switch (spi)
    {
    case F407_SPI1:
        RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
        break;
    case F407_SPI2:
        RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
        break;
    case F407_SPI3:
        RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;
        break;
    default:
        break;
    }
    __DSB();
}

/**
 * @brief 禁用SPI时钟
 */
void f407_spi_clk_disable(f407_spi_t spi)
{
    switch (spi)
    {
    case F407_SPI1:
        RCC->APB2ENR &= ~RCC_APB2ENR_SPI1EN;
        break;
    case F407_SPI2:
        RCC->APB1ENR &= ~RCC_APB1ENR_SPI2EN;
        break;
    case F407_SPI3:
        RCC->APB1ENR &= ~RCC_APB1ENR_SPI3EN;
        break;
    default:
        break;
    }
}

/**
 * @brief 获取SPI外设基地址
 */
SPI_TypeDef *f407_spi_get_base(f407_spi_t spi)
{
    if (spi >= F407_SPI_MAX)
        return NULL;
    return spi_base_table[spi];
}

/*===========================================================================*/
/*                              引脚配置                                      */
/*===========================================================================*/

/**
 * @brief 初始化SPI引脚
 */
int f407_spi_pins_init(f407_spi_pinmap_t pinmap)
{
    switch (pinmap)
    {
    case F407_SPI1_PINS_PA5_PA6_PA7:
        /* SPI1: PA5(SCK), PA6(MISO), PA7(MOSI) - AF5 */
        f407_gpio_config_af(F407_PA5, F407_GPIO_AF5_SPI1_2);
        f407_gpio_config_af(F407_PA6, F407_GPIO_AF5_SPI1_2);
        f407_gpio_config_af(F407_PA7, F407_GPIO_AF5_SPI1_2);
        break;

    case F407_SPI1_PINS_PB3_PB4_PB5:
        /* SPI1: PB3(SCK), PB4(MISO), PB5(MOSI) - AF5 */
        f407_gpio_config_af(F407_PB3, F407_GPIO_AF5_SPI1_2);
        f407_gpio_config_af(F407_PB4, F407_GPIO_AF5_SPI1_2);
        f407_gpio_config_af(F407_PB5, F407_GPIO_AF5_SPI1_2);
        break;

    case F407_SPI2_PINS_PB13_PB14_PB15:
        /* SPI2: PB13(SCK), PB14(MISO), PB15(MOSI) - AF5 */
        f407_gpio_config_af(F407_PB13, F407_GPIO_AF5_SPI1_2);
        f407_gpio_config_af(F407_PB14, F407_GPIO_AF5_SPI1_2);
        f407_gpio_config_af(F407_PB15, F407_GPIO_AF5_SPI1_2);
        break;

    case F407_SPI2_PINS_PB13_PC2_PC3:
        /* SPI2: PB13(SCK), PC2(MISO), PC3(MOSI) - AF5 */
        f407_gpio_config_af(F407_PB13, F407_GPIO_AF5_SPI1_2);
        f407_gpio_config_af(F407_PC2, F407_GPIO_AF5_SPI1_2);
        f407_gpio_config_af(F407_PC3, F407_GPIO_AF5_SPI1_2);
        break;

    case F407_SPI3_PINS_PB3_PB4_PB5:
        /* SPI3: PB3(SCK), PB4(MISO), PB5(MOSI) - AF6 */
        f407_gpio_config_af(F407_PB3, F407_GPIO_AF6_SPI3);
        f407_gpio_config_af(F407_PB4, F407_GPIO_AF6_SPI3);
        f407_gpio_config_af(F407_PB5, F407_GPIO_AF6_SPI3);
        break;

    case F407_SPI3_PINS_PC10_PC11_PC12:
        /* SPI3: PC10(SCK), PC11(MISO), PC12(MOSI) - AF6 */
        f407_gpio_config_af(F407_PC10, F407_GPIO_AF6_SPI3);
        f407_gpio_config_af(F407_PC11, F407_GPIO_AF6_SPI3);
        f407_gpio_config_af(F407_PC12, F407_GPIO_AF6_SPI3);
        break;

    default:
        return -1;
    }

    return 0;
}

/*===========================================================================*/
/*                              初始化函数                                    */
/*===========================================================================*/

/**
 * @brief 初始化SPI
 */
int f407_spi_init(f407_spi_handle_t *handle, const f407_spi_config_t *config)
{
    if (handle == NULL || config == NULL)
        return -1;
    if (config->spi >= F407_SPI_MAX)
        return -1;

    /* 保存配置 */
    handle->config = *config;
    handle->instance = spi_base_table[config->spi];
    handle->initialized = false;

    /* 使能时钟 */
    f407_spi_clk_enable(config->spi);

    /* 初始化引脚 */
    if (f407_spi_pins_init(config->pinmap) != 0)
    {
        return -1;
    }

    SPI_TypeDef *spi = handle->instance;

    /* 禁用SPI进行配置 */
    spi->CR1 &= ~SPI_CR1_SPE;

    /* 配置CR1寄存器 */
    uint32_t cr1 = 0;

    /* 波特率分频 */
    cr1 |= ((uint32_t)config->baudrate << 3);

    /* SPI模式 (CPOL和CPHA) */
    if (config->mode & 0x02)
        cr1 |= SPI_CR1_CPOL;
    if (config->mode & 0x01)
        cr1 |= SPI_CR1_CPHA;

    /* 数据位宽 */
    if (config->datasize == F407_SPI_DATASIZE_16BIT)
    {
        cr1 |= SPI_CR1_DFF;
    }

    /* 主从模式 */
    if (config->role == F407_SPI_MASTER)
    {
        cr1 |= SPI_CR1_MSTR;
    }

    /* 位序 */
    if (config->bitorder == F407_SPI_LSB_FIRST)
    {
        cr1 |= SPI_CR1_LSBFIRST;
    }

    /* NSS管理 */
    if (config->nss == F407_SPI_NSS_SOFT)
    {
        cr1 |= SPI_CR1_SSM | SPI_CR1_SSI;
    }

    spi->CR1 = cr1;

    /* 配置CR2寄存器 (主模式下SSOE = 0) */
    spi->CR2 = 0;

    handle->initialized = true;

    return 0;
}

/**
 * @brief 快速初始化SPI为主机模式
 */
int f407_spi_init_master(f407_spi_handle_t *handle, f407_spi_t spi,
                         f407_spi_baudrate_t baudrate, f407_spi_pinmap_t pinmap)
{
    f407_spi_config_t config = {
        .spi = spi,
        .mode = F407_SPI_MODE0,
        .datasize = F407_SPI_DATASIZE_8BIT,
        .baudrate = baudrate,
        .role = F407_SPI_MASTER,
        .bitorder = F407_SPI_MSB_FIRST,
        .nss = F407_SPI_NSS_SOFT,
        .pinmap = pinmap};

    return f407_spi_init(handle, &config);
}

/**
 * @brief 反初始化SPI
 */
void f407_spi_deinit(f407_spi_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;

    /* 禁用SPI */
    handle->instance->CR1 &= ~SPI_CR1_SPE;

    /* 禁用时钟 */
    f407_spi_clk_disable(handle->config.spi);

    handle->initialized = false;
}

/*===========================================================================*/
/*                              基本操作                                      */
/*===========================================================================*/

/**
 * @brief 使能SPI
 */
void f407_spi_enable(f407_spi_handle_t *handle)
{
    if (handle == NULL)
        return;
    handle->instance->CR1 |= SPI_CR1_SPE;
}

/**
 * @brief 禁用SPI
 */
void f407_spi_disable(f407_spi_handle_t *handle)
{
    if (handle == NULL)
        return;
    handle->instance->CR1 &= ~SPI_CR1_SPE;
}

/**
 * @brief 设置软件CS引脚
 */
void f407_spi_set_cs_pin(f407_spi_handle_t *handle, f407_gpio_id_t cs_pin, bool active_low)
{
    if (handle == NULL)
        return;

    handle->cs_pin = cs_pin;
    handle->cs_active_low = active_low;

    /* 配置CS引脚为推挽输出 */
    f407_gpio_config_output_pp(cs_pin);

    /* 默认取消选中 */
    f407_spi_cs_deselect(handle);
}

/**
 * @brief 选中从设备
 */
void f407_spi_cs_select(f407_spi_handle_t *handle)
{
    if (handle == NULL)
        return;

    if (handle->cs_active_low)
    {
        f407_gpio_reset(handle->cs_pin);
    }
    else
    {
        f407_gpio_set(handle->cs_pin);
    }
}

/**
 * @brief 取消选中从设备
 */
void f407_spi_cs_deselect(f407_spi_handle_t *handle)
{
    if (handle == NULL)
        return;

    if (handle->cs_active_low)
    {
        f407_gpio_set(handle->cs_pin);
    }
    else
    {
        f407_gpio_reset(handle->cs_pin);
    }
}

/*===========================================================================*/
/*                              数据传输                                      */
/*===========================================================================*/

/**
 * @brief 发送单字节
 */
int f407_spi_send_byte(f407_spi_handle_t *handle, uint8_t data)
{
    if (handle == NULL || !handle->initialized)
        return -1;

    SPI_TypeDef *spi = handle->instance;
    uint32_t timeout = F407_SPI_TIMEOUT;

    /* 等待发送缓冲区空 */
    while (!(spi->SR & SPI_SR_TXE))
    {
        if (--timeout == 0)
            return -1;
    }

    /* 发送数据 */
    spi->DR = data;

    /* 等待接收完成 */
    timeout = F407_SPI_TIMEOUT;
    while (!(spi->SR & SPI_SR_RXNE))
    {
        if (--timeout == 0)
            return -1;
    }

    /* 读取数据清除RXNE */
    (void)spi->DR;

    return 0;
}

/**
 * @brief 接收单字节
 */
int f407_spi_receive_byte(f407_spi_handle_t *handle, uint8_t *data)
{
    if (handle == NULL || data == NULL || !handle->initialized)
        return -1;

    *data = f407_spi_transfer_byte(handle, 0xFF);
    return 0;
}

/**
 * @brief 发送并接收单字节
 */
uint8_t f407_spi_transfer_byte(f407_spi_handle_t *handle, uint8_t tx_data)
{
    if (handle == NULL || !handle->initialized)
        return 0;

    SPI_TypeDef *spi = handle->instance;
    uint32_t timeout = F407_SPI_TIMEOUT;

    /* 等待发送缓冲区空 */
    while (!(spi->SR & SPI_SR_TXE))
    {
        if (--timeout == 0)
            return 0;
    }

    /* 发送数据 */
    spi->DR = tx_data;

    /* 等待接收完成 */
    timeout = F407_SPI_TIMEOUT;
    while (!(spi->SR & SPI_SR_RXNE))
    {
        if (--timeout == 0)
            return 0;
    }

    /* 返回接收数据 */
    return (uint8_t)spi->DR;
}

/**
 * @brief 发送数据缓冲区
 */
int f407_spi_send(f407_spi_handle_t *handle, const uint8_t *data, uint16_t len)
{
    if (handle == NULL || data == NULL || !handle->initialized)
        return -1;

    for (uint16_t i = 0; i < len; i++)
    {
        if (f407_spi_send_byte(handle, data[i]) != 0)
        {
            return -1;
        }
    }

    return 0;
}

/**
 * @brief 接收数据到缓冲区
 */
int f407_spi_receive(f407_spi_handle_t *handle, uint8_t *data, uint16_t len)
{
    if (handle == NULL || data == NULL || !handle->initialized)
        return -1;

    for (uint16_t i = 0; i < len; i++)
    {
        data[i] = f407_spi_transfer_byte(handle, 0xFF);
    }

    return 0;
}

/**
 * @brief 全双工传输
 */
int f407_spi_transfer(f407_spi_handle_t *handle, const uint8_t *tx_data,
                      uint8_t *rx_data, uint16_t len)
{
    if (handle == NULL || !handle->initialized)
        return -1;

    for (uint16_t i = 0; i < len; i++)
    {
        uint8_t tx = (tx_data != NULL) ? tx_data[i] : 0xFF;
        uint8_t rx = f407_spi_transfer_byte(handle, tx);
        if (rx_data != NULL)
        {
            rx_data[i] = rx;
        }
    }

    return 0;
}

/**
 * @brief 发送16位数据
 */
int f407_spi_send_16bit(f407_spi_handle_t *handle, uint16_t data)
{
    if (handle == NULL || !handle->initialized)
        return -1;

    SPI_TypeDef *spi = handle->instance;
    uint32_t timeout = F407_SPI_TIMEOUT;

    /* 临时切换到16位模式 */
    uint32_t old_cr1 = spi->CR1;
    spi->CR1 &= ~SPI_CR1_SPE;
    spi->CR1 |= SPI_CR1_DFF;
    spi->CR1 |= SPI_CR1_SPE;

    /* 等待发送缓冲区空 */
    while (!(spi->SR & SPI_SR_TXE))
    {
        if (--timeout == 0)
        {
            spi->CR1 = old_cr1;
            return -1;
        }
    }

    /* 发送数据 */
    spi->DR = data;

    /* 等待接收完成 */
    timeout = F407_SPI_TIMEOUT;
    while (!(spi->SR & SPI_SR_RXNE))
    {
        if (--timeout == 0)
        {
            spi->CR1 = old_cr1;
            return -1;
        }
    }

    /* 读取数据清除RXNE */
    (void)spi->DR;

    /* 恢复原配置 */
    spi->CR1 &= ~SPI_CR1_SPE;
    spi->CR1 = old_cr1;

    return 0;
}

/**
 * @brief 发送并接收16位数据
 */
uint16_t f407_spi_transfer_16bit(f407_spi_handle_t *handle, uint16_t tx_data)
{
    if (handle == NULL || !handle->initialized)
        return 0;

    SPI_TypeDef *spi = handle->instance;
    uint32_t timeout = F407_SPI_TIMEOUT;

    /* 临时切换到16位模式 */
    uint32_t old_cr1 = spi->CR1;
    spi->CR1 &= ~SPI_CR1_SPE;
    spi->CR1 |= SPI_CR1_DFF;
    spi->CR1 |= SPI_CR1_SPE;

    /* 等待发送缓冲区空 */
    while (!(spi->SR & SPI_SR_TXE))
    {
        if (--timeout == 0)
        {
            spi->CR1 = old_cr1;
            return 0;
        }
    }

    /* 发送数据 */
    spi->DR = tx_data;

    /* 等待接收完成 */
    timeout = F407_SPI_TIMEOUT;
    while (!(spi->SR & SPI_SR_RXNE))
    {
        if (--timeout == 0)
        {
            spi->CR1 = old_cr1;
            return 0;
        }
    }

    /* 读取数据 */
    uint16_t rx = (uint16_t)spi->DR;

    /* 恢复原配置 */
    spi->CR1 &= ~SPI_CR1_SPE;
    spi->CR1 = old_cr1;

    return rx;
}

/*===========================================================================*/
/*                              辅助功能                                      */
/*===========================================================================*/

/**
 * @brief 获取SPI波特率
 */
uint32_t f407_spi_get_baudrate(f407_spi_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return 0;

    uint32_t clk = spi_clk_table[handle->config.spi];
    uint32_t br = (handle->instance->CR1 >> 3) & 0x07;

    return clk / (1 << (br + 1));
}

/**
 * @brief 设置SPI波特率
 */
void f407_spi_set_baudrate(f407_spi_handle_t *handle, f407_spi_baudrate_t baudrate)
{
    if (handle == NULL || !handle->initialized)
        return;

    SPI_TypeDef *spi = handle->instance;

    /* 禁用SPI */
    spi->CR1 &= ~SPI_CR1_SPE;

    /* 修改波特率 */
    spi->CR1 &= ~SPI_CR1_BR;
    spi->CR1 |= ((uint32_t)baudrate << 3);

    /* 使能SPI */
    spi->CR1 |= SPI_CR1_SPE;

    handle->config.baudrate = baudrate;
}

/**
 * @brief 检查SPI是否忙
 */
bool f407_spi_is_busy(f407_spi_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return false;
    return (handle->instance->SR & SPI_SR_BSY) ? true : false;
}

/**
 * @brief 等待传输完成
 */
int f407_spi_wait_complete(f407_spi_handle_t *handle, uint32_t timeout)
{
    if (handle == NULL || !handle->initialized)
        return -1;

    while (f407_spi_is_busy(handle))
    {
        if (--timeout == 0)
            return -1;
    }

    return 0;
}

/**
 * @brief 清除SPI所有错误标志
 */
void f407_spi_clear_errors(f407_spi_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;

    /* 读SR和DR清除错误标志 */
    (void)handle->instance->SR;
    (void)handle->instance->DR;
}
