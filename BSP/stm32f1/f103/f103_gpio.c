/**
 * @file f103_gpio.c
 * @brief STM32F103 系列通用 GPIO 驱动实现
 *
 * 提供 STM32F103 系列所有封装型号(C8T6/C6T6/RBT6/VET6等)共享的 GPIO 驱动
 *
 * @note 此驱动适用于所有 STM32F103 系列芯片，与封装无关
 */

#include "f103_gpio.h"

/*===========================================================================*/
/*                            内部数据表                                      */
/*===========================================================================*/

/**
 * @brief GPIO端口基地址表
 */
static GPIO_TypeDef *const gpio_base_table[F103_GPIO_PORT_MAX] = {
    GPIOA, GPIOB, GPIOC, GPIOD, GPIOE};

/**
 * @brief GPIO端口时钟使能位表 (对应 RCC->APB2ENR)
 */
static const uint32_t gpio_clk_table[F103_GPIO_PORT_MAX] = {
    RCC_APB2ENR_IOPAEN,
    RCC_APB2ENR_IOPBEN,
    RCC_APB2ENR_IOPCEN,
    RCC_APB2ENR_IOPDEN,
    RCC_APB2ENR_IOPEEN};

/*===========================================================================*/
/*                            时钟控制                                        */
/*===========================================================================*/

/**
 * @brief 使能GPIO端口时钟
 */
void f103_gpio_clk_enable(f103_gpio_port_t port)
{
    if (port < F103_GPIO_PORT_MAX)
    {
        RCC->APB2ENR |= gpio_clk_table[port];
        /* 添加短暂延时确保时钟稳定 */
        volatile uint32_t tmp = RCC->APB2ENR;
        (void)tmp;
    }
}

/**
 * @brief 禁用GPIO端口时钟
 */
void f103_gpio_clk_disable(f103_gpio_port_t port)
{
    if (port < F103_GPIO_PORT_MAX)
    {
        RCC->APB2ENR &= ~gpio_clk_table[port];
    }
}

/*===========================================================================*/
/*                            GPIO配置                                        */
/*===========================================================================*/

/**
 * @brief 初始化GPIO引脚
 */
int f103_gpio_init(const f103_gpio_config_t *config)
{
    if (config == NULL)
        return -1;

    return f103_gpio_init_quick(config->port, config->pin, config->mode, config->speed);
}

/**
 * @brief 快速初始化GPIO引脚
 */
int f103_gpio_init_quick(f103_gpio_port_t port, f103_gpio_pin_t pin,
                         f103_gpio_mode_t mode, f103_gpio_speed_t speed)
{
    if (port >= F103_GPIO_PORT_MAX || pin >= F103_PIN_MAX)
        return -1;

    GPIO_TypeDef *gpio = gpio_base_table[port];

    /* 使能时钟 */
    f103_gpio_clk_enable(port);

    /* 计算寄存器位置 */
    __IO uint32_t *cr_reg;
    uint32_t pin_offset;

    if (pin < 8)
    {
        cr_reg = &gpio->CRL;
        pin_offset = pin * 4;
    }
    else
    {
        cr_reg = &gpio->CRH;
        pin_offset = (pin - 8) * 4;
    }

    /* 清除原有配置 */
    *cr_reg &= ~(0x0F << pin_offset);

    /* 设置新配置 */
    uint32_t config_val = 0;

    /* 根据模式设置CNF和MODE位 */
    switch (mode)
    {
    case F103_GPIO_MODE_AIN:
        config_val = 0x00; /* CNF=00, MODE=00 */
        break;
    case F103_GPIO_MODE_IN_FLOATING:
        config_val = 0x04; /* CNF=01, MODE=00 */
        break;
    case F103_GPIO_MODE_IPD:
        config_val = 0x08;        /* CNF=10, MODE=00 */
        gpio->ODR &= ~(1 << pin); /* 下拉：ODR=0 */
        break;
    case F103_GPIO_MODE_IPU:
        config_val = 0x08;       /* CNF=10, MODE=00 */
        gpio->ODR |= (1 << pin); /* 上拉：ODR=1 */
        break;
    case F103_GPIO_MODE_OUT_PP:
        config_val = (0x00 | speed); /* CNF=00, MODE=speed */
        break;
    case F103_GPIO_MODE_OUT_OD:
        config_val = (0x04 | speed); /* CNF=01, MODE=speed */
        break;
    case F103_GPIO_MODE_AF_PP:
        config_val = (0x08 | speed); /* CNF=10, MODE=speed */
        break;
    case F103_GPIO_MODE_AF_OD:
        config_val = (0x0C | speed); /* CNF=11, MODE=speed */
        break;
    default:
        return -1;
    }

    *cr_reg |= (config_val << pin_offset);

    return 0;
}

/*===========================================================================*/
/*                            GPIO操作                                        */
/*===========================================================================*/

/**
 * @brief 设置GPIO输出电平
 */
void f103_gpio_write(f103_gpio_port_t port, f103_gpio_pin_t pin, uint8_t level)
{
    if (port >= F103_GPIO_PORT_MAX || pin >= F103_PIN_MAX)
        return;

    GPIO_TypeDef *gpio = gpio_base_table[port];

    if (level)
    {
        gpio->BSRR = (1 << pin);
    }
    else
    {
        gpio->BRR = (1 << pin);
    }
}

/**
 * @brief 读取GPIO输入电平
 */
uint8_t f103_gpio_read(f103_gpio_port_t port, f103_gpio_pin_t pin)
{
    if (port >= F103_GPIO_PORT_MAX || pin >= F103_PIN_MAX)
        return 0;

    GPIO_TypeDef *gpio = gpio_base_table[port];
    return (gpio->IDR & (1 << pin)) ? 1 : 0;
}

/**
 * @brief 翻转GPIO输出电平
 */
void f103_gpio_toggle(f103_gpio_port_t port, f103_gpio_pin_t pin)
{
    if (port >= F103_GPIO_PORT_MAX || pin >= F103_PIN_MAX)
        return;

    GPIO_TypeDef *gpio = gpio_base_table[port];
    gpio->ODR ^= (1 << pin);
}

/**
 * @brief 设置GPIO高电平
 */
void f103_gpio_set(f103_gpio_port_t port, f103_gpio_pin_t pin)
{
    f103_gpio_write(port, pin, 1);
}

/**
 * @brief 设置GPIO低电平
 */
void f103_gpio_reset(f103_gpio_port_t port, f103_gpio_pin_t pin)
{
    f103_gpio_write(port, pin, 0);
}
