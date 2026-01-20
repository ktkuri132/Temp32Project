/**
 * @file f103_i2c.c
 * @brief STM32F103 系列 I2C 驱动实现
 */

#include "f103_i2c.h"
#include "f103_gpio.h"

#define I2C_TIMEOUT 10000

static I2C_TypeDef *const i2c_base_table[F103_I2C_MAX] = {
    I2C1, I2C2};

static void f103_i2c_clk_enable(f103_i2c_port_t port)
{
    switch (port)
    {
    case F103_I2C1:
        RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
        break;
    case F103_I2C2:
        RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
        break;
    default:
        break;
    }
}

static void f103_i2c_gpio_init(f103_i2c_port_t port)
{
    switch (port)
    {
    case F103_I2C1:
        /* PB6: SCL, PB7: SDA */
        f103_gpio_init_quick(F103_GPIOB, F103_PIN_6, F103_GPIO_MODE_AF_OD, F103_GPIO_SPEED_50MHZ);
        f103_gpio_init_quick(F103_GPIOB, F103_PIN_7, F103_GPIO_MODE_AF_OD, F103_GPIO_SPEED_50MHZ);
        break;
    case F103_I2C2:
        /* PB10: SCL, PB11: SDA */
        f103_gpio_init_quick(F103_GPIOB, F103_PIN_10, F103_GPIO_MODE_AF_OD, F103_GPIO_SPEED_50MHZ);
        f103_gpio_init_quick(F103_GPIOB, F103_PIN_11, F103_GPIO_MODE_AF_OD, F103_GPIO_SPEED_50MHZ);
        break;
    default:
        break;
    }
}

int f103_i2c_init(const f103_i2c_config_t *config)
{
    if (config == NULL || config->port >= F103_I2C_MAX)
        return -1;

    I2C_TypeDef *i2c = i2c_base_table[config->port];

    f103_i2c_clk_enable(config->port);
    f103_i2c_gpio_init(config->port);

    /* 禁用I2C */
    i2c->CR1 = 0;

    /* 配置时钟频率 (假设APB1 = 36MHz) */
    uint32_t pclk = 36000000;
    i2c->CR2 = pclk / 1000000; /* FREQ字段 */

    /* 配置CCR */
    if (config->speed == F103_I2C_SPEED_400K)
    {
        /* 快速模式 */
        i2c->CCR = I2C_CCR_FS | (pclk / (config->speed * 3));
        i2c->TRISE = (pclk / 1000000) * 300 / 1000 + 1;
    }
    else
    {
        /* 标准模式 */
        i2c->CCR = pclk / (config->speed * 2);
        i2c->TRISE = pclk / 1000000 + 1;
    }

    /* 使能I2C */
    i2c->CR1 |= I2C_CR1_PE;

    return 0;
}

int f103_i2c_init_quick(f103_i2c_port_t port, f103_i2c_speed_t speed)
{
    f103_i2c_config_t config = {
        .port = port,
        .speed = speed};
    return f103_i2c_init(&config);
}

static int i2c_wait_flag(I2C_TypeDef *i2c, uint32_t flag, uint32_t status)
{
    uint32_t timeout = I2C_TIMEOUT;
    while (((i2c->SR1 & flag) == status) && timeout--)
        ;
    return timeout ? 0 : -1;
}

static int i2c_start(I2C_TypeDef *i2c, uint8_t addr, uint8_t dir)
{
    /* 产生起始条件 */
    i2c->CR1 |= I2C_CR1_START;
    if (i2c_wait_flag(i2c, I2C_SR1_SB, 0) != 0)
        return -1;

    /* 发送地址 */
    i2c->DR = (addr << 1) | dir;
    if (i2c_wait_flag(i2c, I2C_SR1_ADDR, 0) != 0)
        return -1;

    /* 清除ADDR标志 */
    (void)i2c->SR1;
    (void)i2c->SR2;

    return 0;
}

static void i2c_stop(I2C_TypeDef *i2c)
{
    i2c->CR1 |= I2C_CR1_STOP;
}

int f103_i2c_write(f103_i2c_port_t port, uint8_t addr, const uint8_t *data, uint32_t len)
{
    if (port >= F103_I2C_MAX)
        return -1;

    I2C_TypeDef *i2c = i2c_base_table[port];

    if (i2c_start(i2c, addr, 0) != 0)
    {
        i2c_stop(i2c);
        return -1;
    }

    for (uint32_t i = 0; i < len; i++)
    {
        if (i2c_wait_flag(i2c, I2C_SR1_TXE, 0) != 0)
        {
            i2c_stop(i2c);
            return -1;
        }
        i2c->DR = data[i];
    }

    if (i2c_wait_flag(i2c, I2C_SR1_BTF, 0) != 0)
    {
        i2c_stop(i2c);
        return -1;
    }

    i2c_stop(i2c);
    return 0;
}

int f103_i2c_read(f103_i2c_port_t port, uint8_t addr, uint8_t *data, uint32_t len)
{
    if (port >= F103_I2C_MAX || len == 0)
        return -1;

    I2C_TypeDef *i2c = i2c_base_table[port];

    if (len == 1)
    {
        i2c->CR1 &= ~I2C_CR1_ACK;
    }
    else
    {
        i2c->CR1 |= I2C_CR1_ACK;
    }

    if (i2c_start(i2c, addr, 1) != 0)
    {
        i2c_stop(i2c);
        return -1;
    }

    for (uint32_t i = 0; i < len; i++)
    {
        if (i == len - 1)
        {
            i2c->CR1 &= ~I2C_CR1_ACK;
        }

        if (i2c_wait_flag(i2c, I2C_SR1_RXNE, 0) != 0)
        {
            i2c_stop(i2c);
            return -1;
        }
        data[i] = i2c->DR;
    }

    i2c_stop(i2c);
    return 0;
}

int f103_i2c_mem_write(f103_i2c_port_t port, uint8_t addr, uint8_t reg, const uint8_t *data, uint32_t len)
{
    if (port >= F103_I2C_MAX)
        return -1;

    I2C_TypeDef *i2c = i2c_base_table[port];

    if (i2c_start(i2c, addr, 0) != 0)
    {
        i2c_stop(i2c);
        return -1;
    }

    /* 发送寄存器地址 */
    if (i2c_wait_flag(i2c, I2C_SR1_TXE, 0) != 0)
    {
        i2c_stop(i2c);
        return -1;
    }
    i2c->DR = reg;

    /* 发送数据 */
    for (uint32_t i = 0; i < len; i++)
    {
        if (i2c_wait_flag(i2c, I2C_SR1_TXE, 0) != 0)
        {
            i2c_stop(i2c);
            return -1;
        }
        i2c->DR = data[i];
    }

    if (i2c_wait_flag(i2c, I2C_SR1_BTF, 0) != 0)
    {
        i2c_stop(i2c);
        return -1;
    }

    i2c_stop(i2c);
    return 0;
}

int f103_i2c_mem_read(f103_i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t *data, uint32_t len)
{
    if (port >= F103_I2C_MAX)
        return -1;

    I2C_TypeDef *i2c = i2c_base_table[port];

    /* 发送寄存器地址 */
    if (i2c_start(i2c, addr, 0) != 0)
    {
        i2c_stop(i2c);
        return -1;
    }

    if (i2c_wait_flag(i2c, I2C_SR1_TXE, 0) != 0)
    {
        i2c_stop(i2c);
        return -1;
    }
    i2c->DR = reg;

    if (i2c_wait_flag(i2c, I2C_SR1_BTF, 0) != 0)
    {
        i2c_stop(i2c);
        return -1;
    }

    /* 读取数据 */
    return f103_i2c_read(port, addr, data, len);
}

bool f103_i2c_check_device(f103_i2c_port_t port, uint8_t addr)
{
    if (port >= F103_I2C_MAX)
        return false;

    I2C_TypeDef *i2c = i2c_base_table[port];

    if (i2c_start(i2c, addr, 0) != 0)
    {
        i2c_stop(i2c);
        return false;
    }

    i2c_stop(i2c);
    return true;
}
