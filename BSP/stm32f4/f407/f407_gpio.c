/**
 * @file f407_gpio.c
 * @brief STM32F407 系列通用 GPIO 驱动实现
 *
 * 提供 STM32F407 系列所有封装型号(VET6/VGT6/ZET6/ZGT6等)共享的 GPIO 驱动
 *
 * @note 此驱动适用于所有 STM32F407 系列芯片，与封装无关
 */

#include "f407_gpio.h"

/*===========================================================================*/
/*                            内部数据表                                      */
/*===========================================================================*/

/**
 * @brief GPIO端口基地址表
 */
static GPIO_TypeDef *const gpio_base_table[F407_GPIO_PORT_MAX] = {
    GPIOA, GPIOB, GPIOC, GPIOD, GPIOE,
    GPIOF, GPIOG, GPIOH, GPIOI};

/**
 * @brief GPIO端口时钟使能位表 (对应 RCC->AHB1ENR)
 */
static const uint32_t gpio_clk_table[F407_GPIO_PORT_MAX] = {
    RCC_AHB1ENR_GPIOAEN, RCC_AHB1ENR_GPIOBEN, RCC_AHB1ENR_GPIOCEN,
    RCC_AHB1ENR_GPIODEN, RCC_AHB1ENR_GPIOEEN, RCC_AHB1ENR_GPIOFEN,
    RCC_AHB1ENR_GPIOGEN, RCC_AHB1ENR_GPIOHEN, RCC_AHB1ENR_GPIOIEN};

/*===========================================================================*/
/*                            时钟控制                                        */
/*===========================================================================*/

/**
 * @brief 使能GPIO端口时钟
 */
void f407_gpio_clk_enable(f407_gpio_port_t port)
{
    if (port < F407_GPIO_PORT_MAX)
    {
        RCC->AHB1ENR |= gpio_clk_table[port];
        /* 添加短暂延时确保时钟稳定 */
        volatile uint32_t tmp = RCC->AHB1ENR;
        (void)tmp;
    }
}

/**
 * @brief 禁用GPIO端口时钟
 */
void f407_gpio_clk_disable(f407_gpio_port_t port)
{
    if (port < F407_GPIO_PORT_MAX)
    {
        RCC->AHB1ENR &= ~gpio_clk_table[port];
    }
}

/*===========================================================================*/
/*                            GPIO配置                                        */
/*===========================================================================*/

/**
 * @brief 配置GPIO引脚
 */
int f407_gpio_config(const f407_gpio_config_t *cfg)
{
    if (cfg == NULL || cfg->port >= F407_GPIO_PORT_MAX || cfg->pin >= F407_PIN_MAX)
    {
        return -1;
    }

    GPIO_TypeDef *gpio = gpio_base_table[cfg->port];
    uint32_t pin = cfg->pin;
    uint32_t tmp;

    /* 使能GPIO时钟 */
    f407_gpio_clk_enable(cfg->port);

    /* 配置模式 MODER */
    tmp = gpio->MODER;
    tmp &= ~(0x03UL << (pin * 2));
    tmp |= ((uint32_t)cfg->mode << (pin * 2));
    gpio->MODER = tmp;

    /* 配置输出类型 OTYPER */
    tmp = gpio->OTYPER;
    tmp &= ~(0x01UL << pin);
    tmp |= ((uint32_t)cfg->otype << pin);
    gpio->OTYPER = tmp;

    /* 配置速度 OSPEEDR */
    tmp = gpio->OSPEEDR;
    tmp &= ~(0x03UL << (pin * 2));
    tmp |= ((uint32_t)cfg->speed << (pin * 2));
    gpio->OSPEEDR = tmp;

    /* 配置上下拉 PUPDR */
    tmp = gpio->PUPDR;
    tmp &= ~(0x03UL << (pin * 2));
    tmp |= ((uint32_t)cfg->pupd << (pin * 2));
    gpio->PUPDR = tmp;

    /* 配置复用功能 AFR */
    if (cfg->mode == F407_GPIO_MODE_AF)
    {
        uint32_t afr_idx = pin >> 3;         /* AFR[0] for pin 0-7, AFR[1] for pin 8-15 */
        uint32_t afr_pos = (pin & 0x07) * 4; /* 每个引脚占4位 */
        tmp = gpio->AFR[afr_idx];
        tmp &= ~(0x0FUL << afr_pos);
        tmp |= ((uint32_t)cfg->af << afr_pos);
        gpio->AFR[afr_idx] = tmp;
    }

    return 0;
}

/*===========================================================================*/
/*                            快速配置函数                                    */
/*===========================================================================*/

/**
 * @brief 快速配置GPIO引脚为推挽输出
 */
int f407_gpio_config_output_pp(f407_gpio_id_t id)
{
    f407_gpio_config_t cfg = {
        .port = id.port,
        .pin = id.pin,
        .mode = F407_GPIO_MODE_OUTPUT,
        .otype = F407_GPIO_OTYPE_PP,
        .speed = F407_GPIO_SPEED_HIGH,
        .pupd = F407_GPIO_PUPD_NONE,
        .af = F407_GPIO_AF0_SYSTEM};
    return f407_gpio_config(&cfg);
}

/**
 * @brief 快速配置GPIO引脚为开漏输出
 */
int f407_gpio_config_output_od(f407_gpio_id_t id)
{
    f407_gpio_config_t cfg = {
        .port = id.port,
        .pin = id.pin,
        .mode = F407_GPIO_MODE_OUTPUT,
        .otype = F407_GPIO_OTYPE_OD,
        .speed = F407_GPIO_SPEED_HIGH,
        .pupd = F407_GPIO_PUPD_UP,
        .af = F407_GPIO_AF0_SYSTEM};
    return f407_gpio_config(&cfg);
}

/**
 * @brief 快速配置GPIO引脚为浮空输入
 */
int f407_gpio_config_input_floating(f407_gpio_id_t id)
{
    f407_gpio_config_t cfg = {
        .port = id.port,
        .pin = id.pin,
        .mode = F407_GPIO_MODE_INPUT,
        .otype = F407_GPIO_OTYPE_PP,
        .speed = F407_GPIO_SPEED_LOW,
        .pupd = F407_GPIO_PUPD_NONE,
        .af = F407_GPIO_AF0_SYSTEM};
    return f407_gpio_config(&cfg);
}

/**
 * @brief 快速配置GPIO引脚为上拉输入
 */
int f407_gpio_config_input_pu(f407_gpio_id_t id)
{
    f407_gpio_config_t cfg = {
        .port = id.port,
        .pin = id.pin,
        .mode = F407_GPIO_MODE_INPUT,
        .otype = F407_GPIO_OTYPE_PP,
        .speed = F407_GPIO_SPEED_LOW,
        .pupd = F407_GPIO_PUPD_UP,
        .af = F407_GPIO_AF0_SYSTEM};
    return f407_gpio_config(&cfg);
}

/**
 * @brief 快速配置GPIO引脚为下拉输入
 */
int f407_gpio_config_input_pd(f407_gpio_id_t id)
{
    f407_gpio_config_t cfg = {
        .port = id.port,
        .pin = id.pin,
        .mode = F407_GPIO_MODE_INPUT,
        .otype = F407_GPIO_OTYPE_PP,
        .speed = F407_GPIO_SPEED_LOW,
        .pupd = F407_GPIO_PUPD_DOWN,
        .af = F407_GPIO_AF0_SYSTEM};
    return f407_gpio_config(&cfg);
}

/**
 * @brief 快速配置GPIO引脚为模拟模式
 */
int f407_gpio_config_analog(f407_gpio_id_t id)
{
    f407_gpio_config_t cfg = {
        .port = id.port,
        .pin = id.pin,
        .mode = F407_GPIO_MODE_ANALOG,
        .otype = F407_GPIO_OTYPE_PP,
        .speed = F407_GPIO_SPEED_LOW,
        .pupd = F407_GPIO_PUPD_NONE,
        .af = F407_GPIO_AF0_SYSTEM};
    return f407_gpio_config(&cfg);
}

/**
 * @brief 配置GPIO引脚复用功能
 */
int f407_gpio_config_af(f407_gpio_id_t id, f407_gpio_af_t af)
{
    f407_gpio_config_t cfg = {
        .port = id.port,
        .pin = id.pin,
        .mode = F407_GPIO_MODE_AF,
        .otype = F407_GPIO_OTYPE_PP,
        .speed = F407_GPIO_SPEED_HIGH,
        .pupd = F407_GPIO_PUPD_UP,
        .af = af};
    return f407_gpio_config(&cfg);
}

/**
 * @brief 配置GPIO引脚复用功能(带输出类型)
 */
int f407_gpio_config_af_otype(f407_gpio_id_t id, f407_gpio_af_t af, f407_gpio_otype_t otype)
{
    f407_gpio_config_t cfg = {
        .port = id.port,
        .pin = id.pin,
        .mode = F407_GPIO_MODE_AF,
        .otype = otype,
        .speed = F407_GPIO_SPEED_HIGH,
        .pupd = (otype == F407_GPIO_OTYPE_OD) ? F407_GPIO_PUPD_UP : F407_GPIO_PUPD_NONE,
        .af = af};
    return f407_gpio_config(&cfg);
}

/*===========================================================================*/
/*                              输出控制                                      */
/*===========================================================================*/

/**
 * @brief 设置GPIO引脚输出高电平
 */
void f407_gpio_set(f407_gpio_id_t id)
{
    if (id.port >= F407_GPIO_PORT_MAX || id.pin >= F407_PIN_MAX)
        return;
    gpio_base_table[id.port]->BSRRL = (1UL << id.pin);
}

/**
 * @brief 设置GPIO引脚输出低电平
 */
void f407_gpio_reset(f407_gpio_id_t id)
{
    if (id.port >= F407_GPIO_PORT_MAX || id.pin >= F407_PIN_MAX)
        return;
    gpio_base_table[id.port]->BSRRH = (1UL << id.pin);
}

/**
 * @brief 翻转GPIO引脚输出电平
 */
void f407_gpio_toggle(f407_gpio_id_t id)
{
    if (id.port >= F407_GPIO_PORT_MAX || id.pin >= F407_PIN_MAX)
        return;
    gpio_base_table[id.port]->ODR ^= (1UL << id.pin);
}

/**
 * @brief 写GPIO引脚输出电平
 */
void f407_gpio_write(f407_gpio_id_t id, uint8_t state)
{
    if (state)
    {
        f407_gpio_set(id);
    }
    else
    {
        f407_gpio_reset(id);
    }
}

/*===========================================================================*/
/*                              输入读取                                      */
/*===========================================================================*/

/**
 * @brief 读取GPIO引脚输入电平
 */
uint8_t f407_gpio_read(f407_gpio_id_t id)
{
    if (id.port >= F407_GPIO_PORT_MAX || id.pin >= F407_PIN_MAX)
        return 0;
    return (gpio_base_table[id.port]->IDR & (1UL << id.pin)) ? 1 : 0;
}

/**
 * @brief 读取GPIO引脚输出电平
 */
uint8_t f407_gpio_read_output(f407_gpio_id_t id)
{
    if (id.port >= F407_GPIO_PORT_MAX || id.pin >= F407_PIN_MAX)
        return 0;
    return (gpio_base_table[id.port]->ODR & (1UL << id.pin)) ? 1 : 0;
}

/*===========================================================================*/
/*                              端口操作                                      */
/*===========================================================================*/

/**
 * @brief 写GPIO端口整体输出
 */
void f407_gpio_write_port(f407_gpio_port_t port, uint16_t value)
{
    if (port >= F407_GPIO_PORT_MAX)
        return;
    gpio_base_table[port]->ODR = value;
}

/**
 * @brief 读取GPIO端口整体输入
 */
uint16_t f407_gpio_read_port(f407_gpio_port_t port)
{
    if (port >= F407_GPIO_PORT_MAX)
        return 0;
    return (uint16_t)gpio_base_table[port]->IDR;
}

/**
 * @brief 锁定GPIO引脚配置
 */
int f407_gpio_lock(f407_gpio_id_t id)
{
    if (id.port >= F407_GPIO_PORT_MAX || id.pin >= F407_PIN_MAX)
        return -1;

    GPIO_TypeDef *gpio = gpio_base_table[id.port];
    uint32_t pin_mask = (1UL << id.pin);
    uint32_t tmp;

    /* LCKR锁定序列: Write 1 -> Write 0 -> Write 1 -> Read 0 -> Read 1 */
    tmp = (1UL << 16) | pin_mask; /* LCKK = 1 */
    gpio->LCKR = tmp;
    gpio->LCKR = pin_mask; /* LCKK = 0 */
    gpio->LCKR = tmp;      /* LCKK = 1 */
    tmp = gpio->LCKR;      /* Read */
    tmp = gpio->LCKR;      /* Read again */

    /* 验证锁定成功 */
    if ((gpio->LCKR & (1UL << 16)) == 0)
    {
        return -1;
    }

    return 0;
}
