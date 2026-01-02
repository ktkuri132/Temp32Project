/**
 * @file vet6_gpio.c
 * @brief STM32F407VET6 GPIO板级驱动实现
 * @details 提供所有GPIO端口的初始化和操作API
 */

#include "vet6_gpio.h"

/*===========================================================================*/
/*                              内部变量                                      */
/*===========================================================================*/

/* GPIO端口基地址表 */
static GPIO_TypeDef *const gpio_base_table[VET6_GPIO_PORT_MAX] = {
    GPIOA,
    GPIOB,
    GPIOC,
    GPIOD,
    GPIOE};

/* GPIO端口时钟使能位表 */
static const uint32_t gpio_clk_table[VET6_GPIO_PORT_MAX] = {
    RCC_AHB1ENR_GPIOAEN,
    RCC_AHB1ENR_GPIOBEN,
    RCC_AHB1ENR_GPIOCEN,
    RCC_AHB1ENR_GPIODEN,
    RCC_AHB1ENR_GPIOEEN};

/*===========================================================================*/
/*                              时钟控制                                      */
/*===========================================================================*/

/**
 * @brief 使能GPIO端口时钟
 */
void vet6_gpio_clk_enable(vet6_gpio_port_t port)
{
    if (port >= VET6_GPIO_PORT_MAX)
        return;
    RCC->AHB1ENR |= gpio_clk_table[port];

    /* 等待时钟稳定 */
    __DSB();
}

/**
 * @brief 使能所有GPIO端口时钟
 */
void vet6_gpio_clk_enable_all(void)
{
    RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOAEN |
                     RCC_AHB1ENR_GPIOBEN |
                     RCC_AHB1ENR_GPIOCEN |
                     RCC_AHB1ENR_GPIODEN |
                     RCC_AHB1ENR_GPIOEEN);
    __DSB();
}

/**
 * @brief 获取GPIO外设基地址
 */
GPIO_TypeDef *vet6_gpio_get_base(vet6_gpio_port_t port)
{
    if (port >= VET6_GPIO_PORT_MAX)
        return NULL;
    return gpio_base_table[port];
}

/*===========================================================================*/
/*                              配置函数                                      */
/*===========================================================================*/

/**
 * @brief 配置GPIO引脚
 */
int vet6_gpio_config(const vet6_gpio_config_t *config)
{
    if (config == NULL)
        return -1;
    if (config->port >= VET6_GPIO_PORT_MAX)
        return -1;
    if (config->pin >= VET6_PIN_MAX)
        return -1;

    GPIO_TypeDef *gpio = gpio_base_table[config->port];
    uint32_t pin = config->pin;

    /* 使能时钟 */
    vet6_gpio_clk_enable(config->port);

    /* 配置模式 MODER */
    gpio->MODER &= ~(0x3UL << (pin * 2));
    gpio->MODER |= ((uint32_t)config->mode << (pin * 2));

    /* 配置输出类型 OTYPER */
    if (config->otype == VET6_GPIO_OTYPE_OD)
    {
        gpio->OTYPER |= (1UL << pin);
    }
    else
    {
        gpio->OTYPER &= ~(1UL << pin);
    }

    /* 配置速度 OSPEEDR */
    gpio->OSPEEDR &= ~(0x3UL << (pin * 2));
    gpio->OSPEEDR |= ((uint32_t)config->speed << (pin * 2));

    /* 配置上下拉 PUPDR */
    gpio->PUPDR &= ~(0x3UL << (pin * 2));
    gpio->PUPDR |= ((uint32_t)config->pupd << (pin * 2));

    /* 配置复用功能 AFR */
    if (config->mode == VET6_GPIO_MODE_AF)
    {
        if (pin < 8)
        {
            gpio->AFR[0] &= ~(0xFUL << (pin * 4));
            gpio->AFR[0] |= ((uint32_t)config->af << (pin * 4));
        }
        else
        {
            gpio->AFR[1] &= ~(0xFUL << ((pin - 8) * 4));
            gpio->AFR[1] |= ((uint32_t)config->af << ((pin - 8) * 4));
        }
    }

    return 0;
}

/**
 * @brief 快速配置GPIO引脚为推挽输出
 */
int vet6_gpio_config_output_pp(vet6_gpio_id_t id)
{
    vet6_gpio_config_t cfg = {
        .port = id.port,
        .pin = id.pin,
        .mode = VET6_GPIO_MODE_OUTPUT,
        .otype = VET6_GPIO_OTYPE_PP,
        .speed = VET6_GPIO_SPEED_HIGH,
        .pupd = VET6_GPIO_PUPD_NONE,
        .af = VET6_GPIO_AF0_SYSTEM};
    return vet6_gpio_config(&cfg);
}

/**
 * @brief 快速配置GPIO引脚为开漏输出
 */
int vet6_gpio_config_output_od(vet6_gpio_id_t id)
{
    vet6_gpio_config_t cfg = {
        .port = id.port,
        .pin = id.pin,
        .mode = VET6_GPIO_MODE_OUTPUT,
        .otype = VET6_GPIO_OTYPE_OD,
        .speed = VET6_GPIO_SPEED_HIGH,
        .pupd = VET6_GPIO_PUPD_UP,
        .af = VET6_GPIO_AF0_SYSTEM};
    return vet6_gpio_config(&cfg);
}

/**
 * @brief 快速配置GPIO引脚为浮空输入
 */
int vet6_gpio_config_input_float(vet6_gpio_id_t id)
{
    vet6_gpio_config_t cfg = {
        .port = id.port,
        .pin = id.pin,
        .mode = VET6_GPIO_MODE_INPUT,
        .otype = VET6_GPIO_OTYPE_PP,
        .speed = VET6_GPIO_SPEED_LOW,
        .pupd = VET6_GPIO_PUPD_NONE,
        .af = VET6_GPIO_AF0_SYSTEM};
    return vet6_gpio_config(&cfg);
}

/**
 * @brief 快速配置GPIO引脚为上拉输入
 */
int vet6_gpio_config_input_pu(vet6_gpio_id_t id)
{
    vet6_gpio_config_t cfg = {
        .port = id.port,
        .pin = id.pin,
        .mode = VET6_GPIO_MODE_INPUT,
        .otype = VET6_GPIO_OTYPE_PP,
        .speed = VET6_GPIO_SPEED_LOW,
        .pupd = VET6_GPIO_PUPD_UP,
        .af = VET6_GPIO_AF0_SYSTEM};
    return vet6_gpio_config(&cfg);
}

/**
 * @brief 快速配置GPIO引脚为下拉输入
 */
int vet6_gpio_config_input_pd(vet6_gpio_id_t id)
{
    vet6_gpio_config_t cfg = {
        .port = id.port,
        .pin = id.pin,
        .mode = VET6_GPIO_MODE_INPUT,
        .otype = VET6_GPIO_OTYPE_PP,
        .speed = VET6_GPIO_SPEED_LOW,
        .pupd = VET6_GPIO_PUPD_DOWN,
        .af = VET6_GPIO_AF0_SYSTEM};
    return vet6_gpio_config(&cfg);
}

/**
 * @brief 快速配置GPIO引脚为模拟模式
 */
int vet6_gpio_config_analog(vet6_gpio_id_t id)
{
    vet6_gpio_config_t cfg = {
        .port = id.port,
        .pin = id.pin,
        .mode = VET6_GPIO_MODE_ANALOG,
        .otype = VET6_GPIO_OTYPE_PP,
        .speed = VET6_GPIO_SPEED_LOW,
        .pupd = VET6_GPIO_PUPD_NONE,
        .af = VET6_GPIO_AF0_SYSTEM};
    return vet6_gpio_config(&cfg);
}

/**
 * @brief 配置GPIO引脚复用功能
 */
int vet6_gpio_config_af(vet6_gpio_id_t id, vet6_gpio_af_t af)
{
    vet6_gpio_config_t cfg = {
        .port = id.port,
        .pin = id.pin,
        .mode = VET6_GPIO_MODE_AF,
        .otype = VET6_GPIO_OTYPE_PP,
        .speed = VET6_GPIO_SPEED_HIGH,
        .pupd = VET6_GPIO_PUPD_UP,
        .af = af};
    return vet6_gpio_config(&cfg);
}

/**
 * @brief 配置GPIO引脚复用功能(带输出类型)
 */
int vet6_gpio_config_af_otype(vet6_gpio_id_t id, vet6_gpio_af_t af, vet6_gpio_otype_t otype)
{
    vet6_gpio_config_t cfg = {
        .port = id.port,
        .pin = id.pin,
        .mode = VET6_GPIO_MODE_AF,
        .otype = otype,
        .speed = VET6_GPIO_SPEED_HIGH,
        .pupd = (otype == VET6_GPIO_OTYPE_OD) ? VET6_GPIO_PUPD_UP : VET6_GPIO_PUPD_NONE,
        .af = af};
    return vet6_gpio_config(&cfg);
}

/*===========================================================================*/
/*                              输出控制                                      */
/*===========================================================================*/

/**
 * @brief 设置GPIO引脚输出高电平
 */
void vet6_gpio_set(vet6_gpio_id_t id)
{
    if (id.port >= VET6_GPIO_PORT_MAX || id.pin >= VET6_PIN_MAX)
        return;
    gpio_base_table[id.port]->BSRRL = (1UL << id.pin);
}

/**
 * @brief 设置GPIO引脚输出低电平
 */
void vet6_gpio_reset(vet6_gpio_id_t id)
{
    if (id.port >= VET6_GPIO_PORT_MAX || id.pin >= VET6_PIN_MAX)
        return;
    gpio_base_table[id.port]->BSRRH = (1UL << id.pin);
}

/**
 * @brief 翻转GPIO引脚输出电平
 */
void vet6_gpio_toggle(vet6_gpio_id_t id)
{
    if (id.port >= VET6_GPIO_PORT_MAX || id.pin >= VET6_PIN_MAX)
        return;
    gpio_base_table[id.port]->ODR ^= (1UL << id.pin);
}

/**
 * @brief 写GPIO引脚输出电平
 */
void vet6_gpio_write(vet6_gpio_id_t id, uint8_t state)
{
    if (state)
    {
        vet6_gpio_set(id);
    }
    else
    {
        vet6_gpio_reset(id);
    }
}

/*===========================================================================*/
/*                              输入读取                                      */
/*===========================================================================*/

/**
 * @brief 读取GPIO引脚输入电平
 */
uint8_t vet6_gpio_read(vet6_gpio_id_t id)
{
    if (id.port >= VET6_GPIO_PORT_MAX || id.pin >= VET6_PIN_MAX)
        return 0;
    return (gpio_base_table[id.port]->IDR & (1UL << id.pin)) ? 1 : 0;
}

/**
 * @brief 读取GPIO引脚输出电平
 */
uint8_t vet6_gpio_read_output(vet6_gpio_id_t id)
{
    if (id.port >= VET6_GPIO_PORT_MAX || id.pin >= VET6_PIN_MAX)
        return 0;
    return (gpio_base_table[id.port]->ODR & (1UL << id.pin)) ? 1 : 0;
}

/*===========================================================================*/
/*                              端口操作                                      */
/*===========================================================================*/

/**
 * @brief 写GPIO端口整体输出
 */
void vet6_gpio_write_port(vet6_gpio_port_t port, uint16_t value)
{
    if (port >= VET6_GPIO_PORT_MAX)
        return;
    gpio_base_table[port]->ODR = value;
}

/**
 * @brief 读取GPIO端口整体输入
 */
uint16_t vet6_gpio_read_port(vet6_gpio_port_t port)
{
    if (port >= VET6_GPIO_PORT_MAX)
        return 0;
    return (uint16_t)gpio_base_table[port]->IDR;
}

/**
 * @brief 锁定GPIO引脚配置
 */
int vet6_gpio_lock(vet6_gpio_id_t id)
{
    if (id.port >= VET6_GPIO_PORT_MAX || id.pin >= VET6_PIN_MAX)
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
