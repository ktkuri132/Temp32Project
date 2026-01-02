/**
 * @file f407_i2c.c
 * @brief STM32F407系列 I2C板级驱动实现
 * @details 提供I2C1-I2C3的初始化和操作API，支持硬件I2C和软件模拟I2C
 *          适用于STM32F407所有封装型号(VET6/VGT6/ZET6/ZGT6/IGT6等)
 */

#include "f407_i2c.h"

/* 需要外部提供延时函数 */
extern void delay_us(uint32_t us);

/*===========================================================================*/
/*                              内部变量                                      */
/*===========================================================================*/

/* I2C外设基地址表 */
static I2C_TypeDef *const i2c_base_table[F407_I2C_MAX] = {
    I2C1, I2C2, I2C3};

/*===========================================================================*/
/*                              时钟控制                                      */
/*===========================================================================*/

/**
 * @brief 使能I2C时钟
 */
void f407_i2c_clk_enable(f407_i2c_t i2c)
{
    switch (i2c)
    {
    case F407_I2C1:
        RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
        break;
    case F407_I2C2:
        RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
        break;
    case F407_I2C3:
        RCC->APB1ENR |= RCC_APB1ENR_I2C3EN;
        break;
    default:
        break;
    }
    __DSB();
}

/**
 * @brief 禁用I2C时钟
 */
void f407_i2c_clk_disable(f407_i2c_t i2c)
{
    switch (i2c)
    {
    case F407_I2C1:
        RCC->APB1ENR &= ~RCC_APB1ENR_I2C1EN;
        break;
    case F407_I2C2:
        RCC->APB1ENR &= ~RCC_APB1ENR_I2C2EN;
        break;
    case F407_I2C3:
        RCC->APB1ENR &= ~RCC_APB1ENR_I2C3EN;
        break;
    default:
        break;
    }
}

/**
 * @brief 获取I2C外设基地址
 */
I2C_TypeDef *f407_i2c_get_base(f407_i2c_t i2c)
{
    if (i2c >= F407_I2C_MAX)
        return NULL;
    return i2c_base_table[i2c];
}

/*===========================================================================*/
/*                              引脚配置                                      */
/*===========================================================================*/

/**
 * @brief 初始化I2C引脚
 */
int f407_i2c_pins_init(f407_i2c_pinmap_t pinmap)
{
    switch (pinmap)
    {
    case F407_I2C1_PINS_PB6_PB7:
        /* I2C1: PB6(SCL), PB7(SDA) - AF4, 开漏输出 */
        f407_gpio_config_af_otype(F407_PB6, F407_GPIO_AF4_I2C1_3, F407_GPIO_OTYPE_OD);
        f407_gpio_config_af_otype(F407_PB7, F407_GPIO_AF4_I2C1_3, F407_GPIO_OTYPE_OD);
        break;

    case F407_I2C1_PINS_PB8_PB9:
        /* I2C1: PB8(SCL), PB9(SDA) - AF4, 开漏输出 */
        f407_gpio_config_af_otype(F407_PB8, F407_GPIO_AF4_I2C1_3, F407_GPIO_OTYPE_OD);
        f407_gpio_config_af_otype(F407_PB9, F407_GPIO_AF4_I2C1_3, F407_GPIO_OTYPE_OD);
        break;

    case F407_I2C2_PINS_PB10_PB11:
        /* I2C2: PB10(SCL), PB11(SDA) - AF4, 开漏输出 */
        f407_gpio_config_af_otype(F407_PB10, F407_GPIO_AF4_I2C1_3, F407_GPIO_OTYPE_OD);
        f407_gpio_config_af_otype(F407_PB11, F407_GPIO_AF4_I2C1_3, F407_GPIO_OTYPE_OD);
        break;

    case F407_I2C3_PINS_PA8_PC9:
        /* I2C3: PA8(SCL), PC9(SDA) - AF4, 开漏输出 */
        f407_gpio_config_af_otype(F407_PA8, F407_GPIO_AF4_I2C1_3, F407_GPIO_OTYPE_OD);
        f407_gpio_config_af_otype(F407_PC9, F407_GPIO_AF4_I2C1_3, F407_GPIO_OTYPE_OD);
        break;

    default:
        return -1;
    }

    return 0;
}

/*===========================================================================*/
/*                              硬件I2C初始化                                 */
/*===========================================================================*/

/**
 * @brief 初始化硬件I2C
 */
int f407_i2c_init(f407_i2c_handle_t *handle, const f407_i2c_config_t *config)
{
    if (handle == NULL || config == NULL)
        return -1;
    if (config->i2c >= F407_I2C_MAX)
        return -1;

    /* 保存配置 */
    handle->config = *config;
    handle->instance = i2c_base_table[config->i2c];
    handle->initialized = false;

    I2C_TypeDef *i2c = handle->instance;

    /* 使能时钟 */
    f407_i2c_clk_enable(config->i2c);

    /* 初始化引脚 */
    if (f407_i2c_pins_init(config->pinmap) != 0)
    {
        return -1;
    }

    /* 禁用I2C进行配置 */
    i2c->CR1 &= ~I2C_CR1_PE;

    /* 软件复位 */
    i2c->CR1 |= I2C_CR1_SWRST;
    i2c->CR1 &= ~I2C_CR1_SWRST;

    /* 配置时钟频率 */
    uint32_t pclk1 = F407_I2C_APB1_CLK;
    uint32_t freq = pclk1 / 1000000; /* MHz */

    i2c->CR2 = freq & I2C_CR2_FREQ;

    /* 配置时钟控制寄存器 */
    uint32_t ccr;

    if (config->speed <= 100000)
    {
        /* 标准模式 */
        ccr = pclk1 / (config->speed * 2);
        if (ccr < 4)
            ccr = 4;
        i2c->CCR = ccr;
        i2c->TRISE = freq + 1; /* 最大上升时间 1000ns */
    }
    else
    {
        /* 快速模式 */
        ccr = pclk1 / (config->speed * 3); /* 占空比 Tlow/Thigh = 2:1 */
        if (ccr < 1)
            ccr = 1;
        i2c->CCR = ccr | I2C_CCR_FS;
        i2c->TRISE = (freq * 300 / 1000) + 1; /* 最大上升时间 300ns */
    }

    /* 使能I2C */
    i2c->CR1 |= I2C_CR1_PE;

    handle->initialized = true;

    return 0;
}

/**
 * @brief 快速初始化I2C为主机模式
 */
int f407_i2c_init_master(f407_i2c_handle_t *handle, f407_i2c_t i2c,
                         uint32_t speed, f407_i2c_pinmap_t pinmap)
{
    f407_i2c_config_t config = {
        .i2c = i2c,
        .speed = speed,
        .addr_mode = F407_I2C_ADDR_7BIT,
        .pinmap = pinmap,
        .use_dma = false};

    return f407_i2c_init(handle, &config);
}

/**
 * @brief 反初始化I2C
 */
void f407_i2c_deinit(f407_i2c_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;

    /* 禁用I2C */
    handle->instance->CR1 &= ~I2C_CR1_PE;

    /* 禁用时钟 */
    f407_i2c_clk_disable(handle->config.i2c);

    handle->initialized = false;
}

/*===========================================================================*/
/*                              硬件I2C基本操作                               */
/*===========================================================================*/

/**
 * @brief 等待标志位
 */
static int i2c_wait_flag(I2C_TypeDef *i2c, uint32_t flag, uint8_t state, uint32_t timeout)
{
    while (timeout--)
    {
        if (state)
        {
            if (i2c->SR1 & flag)
                return 0;
        }
        else
        {
            if (!(i2c->SR1 & flag))
                return 0;
        }
    }
    return -1;
}

/**
 * @brief 等待SR2标志位
 */
static int i2c_wait_flag_sr2(I2C_TypeDef *i2c, uint32_t flag, uint8_t state, uint32_t timeout)
{
    while (timeout--)
    {
        if (state)
        {
            if (i2c->SR2 & flag)
                return 0;
        }
        else
        {
            if (!(i2c->SR2 & flag))
                return 0;
        }
    }
    return -1;
}

/**
 * @brief I2C发送起始条件
 */
int f407_i2c_start(f407_i2c_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return -1;

    I2C_TypeDef *i2c = handle->instance;

    /* 等待总线空闲 */
    if (i2c_wait_flag_sr2(i2c, I2C_SR2_BUSY, 0, F407_I2C_TIMEOUT) != 0)
    {
        return -1;
    }

    /* 产生起始条件 */
    i2c->CR1 |= I2C_CR1_START;

    /* 等待SB标志 */
    if (i2c_wait_flag(i2c, I2C_SR1_SB, 1, F407_I2C_TIMEOUT) != 0)
    {
        return -1;
    }

    return 0;
}

/**
 * @brief I2C发送停止条件
 */
void f407_i2c_stop(f407_i2c_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;
    handle->instance->CR1 |= I2C_CR1_STOP;
}

/**
 * @brief I2C发送地址
 */
int f407_i2c_send_addr(f407_i2c_handle_t *handle, uint8_t addr, uint8_t dir)
{
    if (handle == NULL || !handle->initialized)
        return -1;

    I2C_TypeDef *i2c = handle->instance;

    /* 发送地址 */
    i2c->DR = (addr << 1) | (dir & 0x01);

    /* 等待ADDR标志 */
    if (i2c_wait_flag(i2c, I2C_SR1_ADDR, 1, F407_I2C_TIMEOUT) != 0)
    {
        /* 检查是否是NACK */
        if (i2c->SR1 & I2C_SR1_AF)
        {
            i2c->SR1 &= ~I2C_SR1_AF;
            f407_i2c_stop(handle);
            return -1;
        }
        return -1;
    }

    /* 清除ADDR标志 (读SR1和SR2) */
    (void)i2c->SR1;
    (void)i2c->SR2;

    return 0;
}

/**
 * @brief I2C发送单字节
 */
int f407_i2c_send_byte(f407_i2c_handle_t *handle, uint8_t data)
{
    if (handle == NULL || !handle->initialized)
        return -1;

    I2C_TypeDef *i2c = handle->instance;

    /* 等待TXE */
    if (i2c_wait_flag(i2c, I2C_SR1_TXE, 1, F407_I2C_TIMEOUT) != 0)
    {
        return -1;
    }

    /* 发送数据 */
    i2c->DR = data;

    /* 等待BTF */
    if (i2c_wait_flag(i2c, I2C_SR1_BTF, 1, F407_I2C_TIMEOUT) != 0)
    {
        return -1;
    }

    return 0;
}

/**
 * @brief I2C接收单字节(带ACK)
 */
int f407_i2c_receive_byte_ack(f407_i2c_handle_t *handle, uint8_t *data)
{
    if (handle == NULL || data == NULL || !handle->initialized)
        return -1;

    I2C_TypeDef *i2c = handle->instance;

    /* 使能ACK */
    i2c->CR1 |= I2C_CR1_ACK;

    /* 等待RXNE */
    if (i2c_wait_flag(i2c, I2C_SR1_RXNE, 1, F407_I2C_TIMEOUT) != 0)
    {
        return -1;
    }

    *data = (uint8_t)i2c->DR;

    return 0;
}

/**
 * @brief I2C接收单字节(带NACK)
 */
int f407_i2c_receive_byte_nack(f407_i2c_handle_t *handle, uint8_t *data)
{
    if (handle == NULL || data == NULL || !handle->initialized)
        return -1;

    I2C_TypeDef *i2c = handle->instance;

    /* 禁用ACK */
    i2c->CR1 &= ~I2C_CR1_ACK;

    /* 等待RXNE */
    if (i2c_wait_flag(i2c, I2C_SR1_RXNE, 1, F407_I2C_TIMEOUT) != 0)
    {
        return -1;
    }

    *data = (uint8_t)i2c->DR;

    return 0;
}

/*===========================================================================*/
/*                              硬件I2C高级操作                               */
/*===========================================================================*/

/**
 * @brief I2C写入数据到设备
 */
int f407_i2c_write(f407_i2c_handle_t *handle, uint8_t dev_addr,
                   const uint8_t *data, uint16_t len)
{
    if (handle == NULL || data == NULL || !handle->initialized)
        return -1;

    /* 发送起始条件 */
    if (f407_i2c_start(handle) != 0)
        return -1;

    /* 发送设备地址+写 */
    if (f407_i2c_send_addr(handle, dev_addr, 0) != 0)
        return -1;

    /* 发送数据 */
    for (uint16_t i = 0; i < len; i++)
    {
        if (f407_i2c_send_byte(handle, data[i]) != 0)
        {
            f407_i2c_stop(handle);
            return -1;
        }
    }

    /* 发送停止条件 */
    f407_i2c_stop(handle);

    return 0;
}

/**
 * @brief I2C从设备读取数据
 */
int f407_i2c_read(f407_i2c_handle_t *handle, uint8_t dev_addr,
                  uint8_t *data, uint16_t len)
{
    if (handle == NULL || data == NULL || len == 0 || !handle->initialized)
        return -1;

    /* 发送起始条件 */
    if (f407_i2c_start(handle) != 0)
        return -1;

    /* 发送设备地址+读 */
    if (f407_i2c_send_addr(handle, dev_addr, 1) != 0)
        return -1;

    /* 接收数据 */
    for (uint16_t i = 0; i < len; i++)
    {
        if (i == len - 1)
        {
            /* 最后一个字节发送NACK */
            if (f407_i2c_receive_byte_nack(handle, &data[i]) != 0)
            {
                f407_i2c_stop(handle);
                return -1;
            }
        }
        else
        {
            if (f407_i2c_receive_byte_ack(handle, &data[i]) != 0)
            {
                f407_i2c_stop(handle);
                return -1;
            }
        }
    }

    /* 发送停止条件 */
    f407_i2c_stop(handle);

    return 0;
}

/**
 * @brief I2C写入数据到设备寄存器
 */
int f407_i2c_mem_write(f407_i2c_handle_t *handle, uint8_t dev_addr,
                       uint8_t reg_addr, const uint8_t *data, uint16_t len)
{
    if (handle == NULL || !handle->initialized)
        return -1;

    /* 发送起始条件 */
    if (f407_i2c_start(handle) != 0)
        return -1;

    /* 发送设备地址+写 */
    if (f407_i2c_send_addr(handle, dev_addr, 0) != 0)
        return -1;

    /* 发送寄存器地址 */
    if (f407_i2c_send_byte(handle, reg_addr) != 0)
    {
        f407_i2c_stop(handle);
        return -1;
    }

    /* 发送数据 */
    if (data != NULL && len > 0)
    {
        for (uint16_t i = 0; i < len; i++)
        {
            if (f407_i2c_send_byte(handle, data[i]) != 0)
            {
                f407_i2c_stop(handle);
                return -1;
            }
        }
    }

    /* 发送停止条件 */
    f407_i2c_stop(handle);

    return 0;
}

/**
 * @brief I2C从设备寄存器读取数据
 */
int f407_i2c_mem_read(f407_i2c_handle_t *handle, uint8_t dev_addr,
                      uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    if (handle == NULL || data == NULL || len == 0 || !handle->initialized)
        return -1;

    I2C_TypeDef *i2c = handle->instance;

    /* 发送起始条件 */
    if (f407_i2c_start(handle) != 0)
        return -1;

    /* 发送设备地址+写 */
    if (f407_i2c_send_addr(handle, dev_addr, 0) != 0)
        return -1;

    /* 发送寄存器地址 */
    if (f407_i2c_send_byte(handle, reg_addr) != 0)
    {
        f407_i2c_stop(handle);
        return -1;
    }

    /* 重新发送起始条件 */
    i2c->CR1 |= I2C_CR1_START;
    if (i2c_wait_flag(i2c, I2C_SR1_SB, 1, F407_I2C_TIMEOUT) != 0)
    {
        return -1;
    }

    /* 发送设备地址+读 */
    if (f407_i2c_send_addr(handle, dev_addr, 1) != 0)
        return -1;

    /* 接收数据 */
    for (uint16_t i = 0; i < len; i++)
    {
        if (i == len - 1)
        {
            if (f407_i2c_receive_byte_nack(handle, &data[i]) != 0)
            {
                f407_i2c_stop(handle);
                return -1;
            }
        }
        else
        {
            if (f407_i2c_receive_byte_ack(handle, &data[i]) != 0)
            {
                f407_i2c_stop(handle);
                return -1;
            }
        }
    }

    /* 发送停止条件 */
    f407_i2c_stop(handle);

    return 0;
}

/**
 * @brief 检测I2C设备是否存在
 */
bool f407_i2c_device_ready(f407_i2c_handle_t *handle, uint8_t dev_addr)
{
    if (handle == NULL || !handle->initialized)
        return false;

    /* 发送起始条件 */
    if (f407_i2c_start(handle) != 0)
        return false;

    /* 发送设备地址+写 */
    int ret = f407_i2c_send_addr(handle, dev_addr, 0);

    /* 发送停止条件 */
    f407_i2c_stop(handle);

    return (ret == 0);
}

/**
 * @brief 扫描I2C总线上的设备
 */
uint8_t f407_i2c_scan(f407_i2c_handle_t *handle, uint8_t *addr_list, uint8_t max_count)
{
    if (handle == NULL || addr_list == NULL || !handle->initialized)
        return 0;

    uint8_t count = 0;

    for (uint8_t addr = 0x08; addr < 0x78 && count < max_count; addr++)
    {
        if (f407_i2c_device_ready(handle, addr))
        {
            addr_list[count++] = addr;
        }
    }

    return count;
}

/**
 * @brief 复位I2C总线
 */
void f407_i2c_reset(f407_i2c_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;

    I2C_TypeDef *i2c = handle->instance;

    /* 禁用I2C */
    i2c->CR1 &= ~I2C_CR1_PE;

    /* 软件复位 */
    i2c->CR1 |= I2C_CR1_SWRST;
    i2c->CR1 &= ~I2C_CR1_SWRST;

    /* 重新初始化 */
    f407_i2c_init(handle, &handle->config);
}

/*===========================================================================*/
/*                              软件I2C实现                                   */
/*===========================================================================*/

/* 软件I2C延时 */
static void soft_i2c_delay(f407_soft_i2c_handle_t *handle)
{
    delay_us(handle->delay_us);
}

/* 设置SCL */
static void soft_i2c_scl(f407_soft_i2c_handle_t *handle, uint8_t state)
{
    f407_gpio_write(handle->scl_pin, state);
}

/* 设置SDA */
static void soft_i2c_sda(f407_soft_i2c_handle_t *handle, uint8_t state)
{
    f407_gpio_write(handle->sda_pin, state);
}

/* 读取SDA */
static uint8_t soft_i2c_read_sda(f407_soft_i2c_handle_t *handle)
{
    return f407_gpio_read(handle->sda_pin);
}

/* SDA设置为输入 */
static void soft_i2c_sda_input(f407_soft_i2c_handle_t *handle)
{
    f407_gpio_config_input_pu(handle->sda_pin);
}

/* SDA设置为输出 */
static void soft_i2c_sda_output(f407_soft_i2c_handle_t *handle)
{
    f407_gpio_config_output_od(handle->sda_pin);
}

/**
 * @brief 初始化软件I2C
 */
int f407_soft_i2c_init(f407_soft_i2c_handle_t *handle, const f407_soft_i2c_config_t *config)
{
    if (handle == NULL || config == NULL)
        return -1;

    handle->scl_pin = config->scl_pin;
    handle->sda_pin = config->sda_pin;
    handle->delay_us = (config->delay_us > 0) ? config->delay_us : 2;

    /* 配置GPIO为开漏输出 */
    f407_gpio_config_output_od(handle->scl_pin);
    f407_gpio_config_output_od(handle->sda_pin);

    /* 默认空闲状态:都为高电平 */
    f407_gpio_set(handle->scl_pin);
    f407_gpio_set(handle->sda_pin);

    handle->initialized = true;

    return 0;
}

/**
 * @brief 快速初始化软件I2C
 */
int f407_soft_i2c_init_quick(f407_soft_i2c_handle_t *handle,
                             f407_gpio_id_t scl_pin, f407_gpio_id_t sda_pin)
{
    f407_soft_i2c_config_t config = {
        .scl_pin = scl_pin,
        .sda_pin = sda_pin,
        .delay_us = 2};

    return f407_soft_i2c_init(handle, &config);
}

/**
 * @brief 软件I2C发送起始条件
 */
void f407_soft_i2c_start(f407_soft_i2c_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;

    soft_i2c_sda_output(handle);
    soft_i2c_sda(handle, 1);
    soft_i2c_scl(handle, 1);
    soft_i2c_delay(handle);
    soft_i2c_sda(handle, 0); /* SCL高时SDA下降沿 = 起始 */
    soft_i2c_delay(handle);
    soft_i2c_scl(handle, 0); /* 钳住总线 */
}

/**
 * @brief 软件I2C发送停止条件
 */
void f407_soft_i2c_stop(f407_soft_i2c_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;

    soft_i2c_sda_output(handle);
    soft_i2c_scl(handle, 0);
    soft_i2c_sda(handle, 0);
    soft_i2c_delay(handle);
    soft_i2c_scl(handle, 1);
    soft_i2c_delay(handle);
    soft_i2c_sda(handle, 1); /* SCL高时SDA上升沿 = 停止 */
    soft_i2c_delay(handle);
}

/**
 * @brief 软件I2C发送单字节
 */
uint8_t f407_soft_i2c_send_byte(f407_soft_i2c_handle_t *handle, uint8_t data)
{
    if (handle == NULL || !handle->initialized)
        return 1;

    soft_i2c_sda_output(handle);

    /* 发送8位数据 */
    for (uint8_t i = 0; i < 8; i++)
    {
        soft_i2c_scl(handle, 0);
        soft_i2c_delay(handle);

        if (data & 0x80)
        {
            soft_i2c_sda(handle, 1);
        }
        else
        {
            soft_i2c_sda(handle, 0);
        }
        data <<= 1;

        soft_i2c_delay(handle);
        soft_i2c_scl(handle, 1);
        soft_i2c_delay(handle);
    }

    /* 接收ACK */
    soft_i2c_scl(handle, 0);
    soft_i2c_sda(handle, 1);
    soft_i2c_delay(handle);
    soft_i2c_sda_input(handle);
    soft_i2c_delay(handle);
    soft_i2c_scl(handle, 1);
    soft_i2c_delay(handle);

    uint8_t ack = soft_i2c_read_sda(handle);

    soft_i2c_scl(handle, 0);
    soft_i2c_sda_output(handle);

    return ack;
}

/**
 * @brief 软件I2C接收单字节
 */
uint8_t f407_soft_i2c_receive_byte(f407_soft_i2c_handle_t *handle, bool ack)
{
    if (handle == NULL || !handle->initialized)
        return 0;

    uint8_t data = 0;

    soft_i2c_sda_input(handle);

    /* 接收8位数据 */
    for (uint8_t i = 0; i < 8; i++)
    {
        soft_i2c_scl(handle, 0);
        soft_i2c_delay(handle);
        soft_i2c_scl(handle, 1);
        soft_i2c_delay(handle);

        data <<= 1;
        if (soft_i2c_read_sda(handle))
        {
            data |= 0x01;
        }
    }

    /* 发送ACK/NACK */
    soft_i2c_scl(handle, 0);
    soft_i2c_sda_output(handle);
    soft_i2c_delay(handle);

    if (ack)
    {
        soft_i2c_sda(handle, 0); /* ACK */
    }
    else
    {
        soft_i2c_sda(handle, 1); /* NACK */
    }

    soft_i2c_delay(handle);
    soft_i2c_scl(handle, 1);
    soft_i2c_delay(handle);
    soft_i2c_scl(handle, 0);

    return data;
}

/**
 * @brief 软件I2C写入数据到设备寄存器
 */
int f407_soft_i2c_mem_write(f407_soft_i2c_handle_t *handle, uint8_t dev_addr,
                            uint8_t reg_addr, const uint8_t *data, uint16_t len)
{
    if (handle == NULL || !handle->initialized)
        return -1;

    f407_soft_i2c_start(handle);

    /* 发送设备地址+写 */
    if (f407_soft_i2c_send_byte(handle, (dev_addr << 1) | 0) != 0)
    {
        f407_soft_i2c_stop(handle);
        return -1;
    }

    /* 发送寄存器地址 */
    if (f407_soft_i2c_send_byte(handle, reg_addr) != 0)
    {
        f407_soft_i2c_stop(handle);
        return -1;
    }

    /* 发送数据 */
    if (data != NULL && len > 0)
    {
        for (uint16_t i = 0; i < len; i++)
        {
            if (f407_soft_i2c_send_byte(handle, data[i]) != 0)
            {
                f407_soft_i2c_stop(handle);
                return -1;
            }
        }
    }

    f407_soft_i2c_stop(handle);

    return 0;
}

/**
 * @brief 软件I2C从设备寄存器读取数据
 */
int f407_soft_i2c_mem_read(f407_soft_i2c_handle_t *handle, uint8_t dev_addr,
                           uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    if (handle == NULL || data == NULL || len == 0 || !handle->initialized)
        return -1;

    f407_soft_i2c_start(handle);

    /* 发送设备地址+写 */
    if (f407_soft_i2c_send_byte(handle, (dev_addr << 1) | 0) != 0)
    {
        f407_soft_i2c_stop(handle);
        return -1;
    }

    /* 发送寄存器地址 */
    if (f407_soft_i2c_send_byte(handle, reg_addr) != 0)
    {
        f407_soft_i2c_stop(handle);
        return -1;
    }

    /* 重新起始 */
    f407_soft_i2c_start(handle);

    /* 发送设备地址+读 */
    if (f407_soft_i2c_send_byte(handle, (dev_addr << 1) | 1) != 0)
    {
        f407_soft_i2c_stop(handle);
        return -1;
    }

    /* 接收数据 */
    for (uint16_t i = 0; i < len; i++)
    {
        data[i] = f407_soft_i2c_receive_byte(handle, (i < len - 1));
    }

    f407_soft_i2c_stop(handle);

    return 0;
}

/**
 * @brief 软件I2C检测设备
 */
bool f407_soft_i2c_device_ready(f407_soft_i2c_handle_t *handle, uint8_t dev_addr)
{
    if (handle == NULL || !handle->initialized)
        return false;

    f407_soft_i2c_start(handle);

    uint8_t ack = f407_soft_i2c_send_byte(handle, (dev_addr << 1) | 0);

    f407_soft_i2c_stop(handle);

    return (ack == 0);
}
