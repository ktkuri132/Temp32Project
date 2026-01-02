/**
 * @file f407_usart.c
 * @brief STM32F407 系列通用 USART 驱动实现
 * @details 提供USART1-USART6的初始化和操作API
 *
 * @note 此驱动适用于所有 STM32F407 系列芯片，与封装无关
 */

#include "f407_usart.h"
#include <stdio.h>
#include <string.h>

/*===========================================================================*/
/*                              内部变量                                      */
/*===========================================================================*/

/* USART外设基地址表 */
static USART_TypeDef *const usart_base_table[F407_USART_MAX] = {
    USART1, USART2, USART3, UART4, UART5, USART6};

/* USART中断号表 */
static const IRQn_Type usart_irq_table[F407_USART_MAX] = {
    USART1_IRQn, USART2_IRQn, USART3_IRQn,
    UART4_IRQn, UART5_IRQn, USART6_IRQn};

/* USART时钟表 */
static const uint32_t usart_clk_table[F407_USART_MAX] = {
    F407_USART1_CLK, F407_USART2_CLK, F407_USART3_CLK,
    F407_UART4_CLK, F407_UART5_CLK, F407_USART6_CLK};

/*===========================================================================*/
/*                              时钟控制                                      */
/*===========================================================================*/

/**
 * @brief 使能USART时钟
 */
void f407_usart_clk_enable(f407_usart_t usart)
{
    switch (usart)
    {
    case F407_USART1:
        RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
        break;
    case F407_USART2:
        RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
        break;
    case F407_USART3:
        RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
        break;
    case F407_UART4:
        RCC->APB1ENR |= RCC_APB1ENR_UART4EN;
        break;
    case F407_UART5:
        RCC->APB1ENR |= RCC_APB1ENR_UART5EN;
        break;
    case F407_USART6:
        RCC->APB2ENR |= RCC_APB2ENR_USART6EN;
        break;
    default:
        break;
    }
    __DSB();
}

/**
 * @brief 禁用USART时钟
 */
void f407_usart_clk_disable(f407_usart_t usart)
{
    switch (usart)
    {
    case F407_USART1:
        RCC->APB2ENR &= ~RCC_APB2ENR_USART1EN;
        break;
    case F407_USART2:
        RCC->APB1ENR &= ~RCC_APB1ENR_USART2EN;
        break;
    case F407_USART3:
        RCC->APB1ENR &= ~RCC_APB1ENR_USART3EN;
        break;
    case F407_UART4:
        RCC->APB1ENR &= ~RCC_APB1ENR_UART4EN;
        break;
    case F407_UART5:
        RCC->APB1ENR &= ~RCC_APB1ENR_UART5EN;
        break;
    case F407_USART6:
        RCC->APB2ENR &= ~RCC_APB2ENR_USART6EN;
        break;
    default:
        break;
    }
}

/**
 * @brief 获取USART外设基地址
 */
USART_TypeDef *f407_usart_get_base(f407_usart_t usart)
{
    if (usart >= F407_USART_MAX)
        return NULL;
    return usart_base_table[usart];
}

/**
 * @brief 获取USART时钟频率
 */
uint32_t f407_usart_get_clk(f407_usart_t usart)
{
    if (usart >= F407_USART_MAX)
        return 0;
    return usart_clk_table[usart];
}

/*===========================================================================*/
/*                              引脚配置                                      */
/*===========================================================================*/

/**
 * @brief 初始化USART引脚
 */
int f407_usart_pins_init(f407_usart_pinmap_t pinmap)
{
    switch (pinmap)
    {
    case F407_USART1_PINS_PA9_PA10:
        /* USART1: PA9(TX), PA10(RX) - AF7 */
        f407_gpio_config_af(F407_PA9, F407_GPIO_AF7_USART1_3);
        f407_gpio_config_af(F407_PA10, F407_GPIO_AF7_USART1_3);
        break;

    case F407_USART1_PINS_PB6_PB7:
        /* USART1: PB6(TX), PB7(RX) - AF7 */
        f407_gpio_config_af(F407_PB6, F407_GPIO_AF7_USART1_3);
        f407_gpio_config_af(F407_PB7, F407_GPIO_AF7_USART1_3);
        break;

    case F407_USART2_PINS_PA2_PA3:
        /* USART2: PA2(TX), PA3(RX) - AF7 */
        f407_gpio_config_af(F407_PA2, F407_GPIO_AF7_USART1_3);
        f407_gpio_config_af(F407_PA3, F407_GPIO_AF7_USART1_3);
        break;

    case F407_USART2_PINS_PD5_PD6:
        /* USART2: PD5(TX), PD6(RX) - AF7 */
        f407_gpio_config_af(F407_PD5, F407_GPIO_AF7_USART1_3);
        f407_gpio_config_af(F407_PD6, F407_GPIO_AF7_USART1_3);
        break;

    case F407_USART3_PINS_PB10_PB11:
        /* USART3: PB10(TX), PB11(RX) - AF7 */
        f407_gpio_config_af(F407_PB10, F407_GPIO_AF7_USART1_3);
        f407_gpio_config_af(F407_PB11, F407_GPIO_AF7_USART1_3);
        break;

    case F407_USART3_PINS_PC10_PC11:
        /* USART3: PC10(TX), PC11(RX) - AF7 */
        f407_gpio_config_af(F407_PC10, F407_GPIO_AF7_USART1_3);
        f407_gpio_config_af(F407_PC11, F407_GPIO_AF7_USART1_3);
        break;

    case F407_USART3_PINS_PD8_PD9:
        /* USART3: PD8(TX), PD9(RX) - AF7 */
        f407_gpio_config_af(F407_PD8, F407_GPIO_AF7_USART1_3);
        f407_gpio_config_af(F407_PD9, F407_GPIO_AF7_USART1_3);
        break;

    case F407_UART4_PINS_PA0_PA1:
        /* UART4: PA0(TX), PA1(RX) - AF8 */
        f407_gpio_config_af(F407_PA0, F407_GPIO_AF8_USART4_6);
        f407_gpio_config_af(F407_PA1, F407_GPIO_AF8_USART4_6);
        break;

    case F407_UART4_PINS_PC10_PC11:
        /* UART4: PC10(TX), PC11(RX) - AF8 */
        f407_gpio_config_af(F407_PC10, F407_GPIO_AF8_USART4_6);
        f407_gpio_config_af(F407_PC11, F407_GPIO_AF8_USART4_6);
        break;

    case F407_UART5_PINS_PC12_PD2:
        /* UART5: PC12(TX), PD2(RX) - AF8 */
        f407_gpio_config_af(F407_PC12, F407_GPIO_AF8_USART4_6);
        f407_gpio_config_af(F407_PD2, F407_GPIO_AF8_USART4_6);
        break;

    case F407_USART6_PINS_PC6_PC7:
        /* USART6: PC6(TX), PC7(RX) - AF8 */
        f407_gpio_config_af(F407_PC6, F407_GPIO_AF8_USART4_6);
        f407_gpio_config_af(F407_PC7, F407_GPIO_AF8_USART4_6);
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
 * @brief 初始化USART
 */
int f407_usart_init(f407_usart_handle_t *handle, const f407_usart_config_t *config)
{
    if (handle == NULL || config == NULL)
        return -1;
    if (config->usart >= F407_USART_MAX)
        return -1;

    /* 保存配置 */
    handle->config = *config;
    handle->instance = usart_base_table[config->usart];
    handle->initialized = false;
    handle->rx_callback = NULL;
    handle->rx_buffer = NULL;
    handle->rx_size = 0;
    handle->rx_head = 0;
    handle->rx_tail = 0;

    USART_TypeDef *usart = handle->instance;

    /* 使能时钟 */
    f407_usart_clk_enable(config->usart);

    /* 初始化引脚 */
    if (f407_usart_pins_init(config->pinmap) != 0)
    {
        return -1;
    }

    /* 禁用USART进行配置 */
    usart->CR1 &= ~USART_CR1_UE;

    /* 配置波特率 */
    uint32_t clk = usart_clk_table[config->usart];
    usart->BRR = clk / config->baudrate;

    /* 配置CR1 */
    uint32_t cr1 = 0;

    /* 数据位 */
    if (config->wordlen == F407_USART_WORDLEN_9B)
    {
        cr1 |= USART_CR1_M;
    }

    /* 校验位 */
    if (config->parity != F407_USART_PARITY_NONE)
    {
        cr1 |= USART_CR1_PCE;
        if (config->parity == F407_USART_PARITY_ODD)
        {
            cr1 |= USART_CR1_PS;
        }
    }

    /* 发送/接收使能 */
    if (config->mode & F407_USART_MODE_TX)
    {
        cr1 |= USART_CR1_TE;
    }
    if (config->mode & F407_USART_MODE_RX)
    {
        cr1 |= USART_CR1_RE;
    }

    usart->CR1 = cr1;

    /* 配置CR2 - 停止位 */
    usart->CR2 &= ~USART_CR2_STOP;
    usart->CR2 |= ((uint32_t)config->stopbits << 12);

    /* 配置CR3 - 硬件流控 */
    usart->CR3 &= ~(USART_CR3_RTSE | USART_CR3_CTSE);
    if (config->hwflow & F407_USART_HWFLOW_RTS)
    {
        usart->CR3 |= USART_CR3_RTSE;
    }
    if (config->hwflow & F407_USART_HWFLOW_CTS)
    {
        usart->CR3 |= USART_CR3_CTSE;
    }

    /* 使能USART */
    usart->CR1 |= USART_CR1_UE;

    handle->initialized = true;

    return 0;
}

/**
 * @brief 快速初始化USART (8N1)
 */
int f407_usart_init_simple(f407_usart_handle_t *handle, f407_usart_t usart,
                           uint32_t baudrate, f407_usart_pinmap_t pinmap)
{
    f407_usart_config_t config = {
        .usart = usart,
        .baudrate = baudrate,
        .wordlen = F407_USART_WORDLEN_8B,
        .stopbits = F407_USART_STOPBITS_1,
        .parity = F407_USART_PARITY_NONE,
        .mode = F407_USART_MODE_TX_RX,
        .hwflow = F407_USART_HWFLOW_NONE,
        .pinmap = pinmap};

    return f407_usart_init(handle, &config);
}

/**
 * @brief 反初始化USART
 */
void f407_usart_deinit(f407_usart_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;

    /* 禁用USART */
    handle->instance->CR1 &= ~USART_CR1_UE;

    /* 禁用中断 */
    NVIC_DisableIRQ(usart_irq_table[handle->config.usart]);

    /* 禁用时钟 */
    f407_usart_clk_disable(handle->config.usart);

    handle->initialized = false;
}

/*===========================================================================*/
/*                              基本操作                                      */
/*===========================================================================*/

/**
 * @brief 使能USART
 */
void f407_usart_enable(f407_usart_handle_t *handle)
{
    if (handle == NULL)
        return;
    handle->instance->CR1 |= USART_CR1_UE;
}

/**
 * @brief 禁用USART
 */
void f407_usart_disable(f407_usart_handle_t *handle)
{
    if (handle == NULL)
        return;
    handle->instance->CR1 &= ~USART_CR1_UE;
}

/**
 * @brief 发送单字节
 */
int f407_usart_send_byte(f407_usart_handle_t *handle, uint8_t data)
{
    if (handle == NULL || !handle->initialized)
        return -1;

    USART_TypeDef *usart = handle->instance;
    uint32_t timeout = F407_USART_TIMEOUT;

    /* 等待TXE */
    while (!(usart->SR & USART_SR_TXE))
    {
        if (--timeout == 0)
            return -1;
    }

    usart->DR = data;

    return 0;
}

/**
 * @brief 接收单字节(阻塞)
 */
int f407_usart_receive_byte(f407_usart_handle_t *handle, uint8_t *data, uint32_t timeout)
{
    if (handle == NULL || data == NULL || !handle->initialized)
        return -1;

    USART_TypeDef *usart = handle->instance;

    /* 等待RXNE */
    while (!(usart->SR & USART_SR_RXNE))
    {
        if (--timeout == 0)
            return -1;
    }

    *data = (uint8_t)usart->DR;

    return 0;
}

/**
 * @brief 发送数据缓冲区
 */
int f407_usart_send(f407_usart_handle_t *handle, const uint8_t *data, uint16_t len)
{
    if (handle == NULL || data == NULL || !handle->initialized)
        return -1;

    for (uint16_t i = 0; i < len; i++)
    {
        if (f407_usart_send_byte(handle, data[i]) != 0)
        {
            return -1;
        }
    }

    return 0;
}

/**
 * @brief 发送字符串
 */
int f407_usart_send_string(f407_usart_handle_t *handle, const char *str)
{
    if (handle == NULL || str == NULL || !handle->initialized)
        return -1;

    while (*str)
    {
        if (f407_usart_send_byte(handle, *str++) != 0)
        {
            return -1;
        }
    }

    return 0;
}

/**
 * @brief 接收数据到缓冲区(阻塞)
 */
int f407_usart_receive(f407_usart_handle_t *handle, uint8_t *data, uint16_t len, uint32_t timeout)
{
    if (handle == NULL || data == NULL || !handle->initialized)
        return 0;

    uint16_t received = 0;

    while (received < len)
    {
        if (f407_usart_receive_byte(handle, &data[received], timeout) != 0)
        {
            break;
        }
        received++;
    }

    return received;
}

/**
 * @brief 格式化打印
 */
int f407_usart_printf(f407_usart_handle_t *handle, const char *format, ...)
{
    if (handle == NULL || format == NULL || !handle->initialized)
        return -1;

    char buffer[256];
    va_list args;

    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (len > 0)
    {
        f407_usart_send(handle, (uint8_t *)buffer, len);
    }

    return len;
}

/*===========================================================================*/
/*                              中断相关                                      */
/*===========================================================================*/

/**
 * @brief 使能USART接收中断
 */
void f407_usart_enable_rx_irq(f407_usart_handle_t *handle, uint8_t priority)
{
    if (handle == NULL || !handle->initialized)
        return;

    /* 清除挂起标志 */
    (void)handle->instance->SR;
    (void)handle->instance->DR;

    /* 使能RXNE中断 */
    handle->instance->CR1 |= USART_CR1_RXNEIE;

    /* 配置NVIC */
    NVIC_SetPriority(usart_irq_table[handle->config.usart], priority);
    NVIC_EnableIRQ(usart_irq_table[handle->config.usart]);
}

/**
 * @brief 禁用USART接收中断
 */
void f407_usart_disable_rx_irq(f407_usart_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;

    handle->instance->CR1 &= ~USART_CR1_RXNEIE;
    NVIC_DisableIRQ(usart_irq_table[handle->config.usart]);
}

/**
 * @brief 设置接收回调函数
 */
void f407_usart_set_rx_callback(f407_usart_handle_t *handle, f407_usart_rx_callback_t callback)
{
    if (handle == NULL)
        return;
    handle->rx_callback = callback;
}

/**
 * @brief 设置接收缓冲区
 */
void f407_usart_set_rx_buffer(f407_usart_handle_t *handle, uint8_t *buffer, uint16_t size)
{
    if (handle == NULL)
        return;

    handle->rx_buffer = buffer;
    handle->rx_size = size;
    handle->rx_head = 0;
    handle->rx_tail = 0;
}

/**
 * @brief 从接收缓冲区读取数据
 */
int f407_usart_read_buffer(f407_usart_handle_t *handle, uint8_t *data, uint16_t len)
{
    if (handle == NULL || data == NULL || handle->rx_buffer == NULL)
        return 0;

    uint16_t count = 0;

    while (count < len && handle->rx_tail != handle->rx_head)
    {
        data[count++] = handle->rx_buffer[handle->rx_tail];
        handle->rx_tail = (handle->rx_tail + 1) % handle->rx_size;
    }

    return count;
}

/**
 * @brief 获取接收缓冲区数据量
 */
uint16_t f407_usart_available(f407_usart_handle_t *handle)
{
    if (handle == NULL || handle->rx_buffer == NULL)
        return 0;

    if (handle->rx_head >= handle->rx_tail)
    {
        return handle->rx_head - handle->rx_tail;
    }
    else
    {
        return handle->rx_size - handle->rx_tail + handle->rx_head;
    }
}

/**
 * @brief 清空接收缓冲区
 */
void f407_usart_flush_rx(f407_usart_handle_t *handle)
{
    if (handle == NULL)
        return;

    handle->rx_head = 0;
    handle->rx_tail = 0;

    /* 清除硬件接收缓冲区 */
    (void)handle->instance->DR;
}

/**
 * @brief USART中断处理
 */
void f407_usart_irq_handler(f407_usart_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;

    USART_TypeDef *usart = handle->instance;
    uint32_t sr = usart->SR;

    /* 接收中断 */
    if (sr & USART_SR_RXNE)
    {
        uint8_t data = (uint8_t)usart->DR;

        /* 调用回调函数 */
        if (handle->rx_callback != NULL)
        {
            handle->rx_callback(data);
        }

        /* 写入环形缓冲区 */
        if (handle->rx_buffer != NULL)
        {
            uint16_t next = (handle->rx_head + 1) % handle->rx_size;
            if (next != handle->rx_tail)
            {
                handle->rx_buffer[handle->rx_head] = data;
                handle->rx_head = next;
            }
            /* 缓冲区满则丢弃数据 */
        }
    }

    /* 溢出错误处理 */
    if (sr & USART_SR_ORE)
    {
        (void)usart->DR; /* 读DR清除ORE */
    }
}

/*===========================================================================*/
/*                              辅助功能                                      */
/*===========================================================================*/

/**
 * @brief 设置波特率
 */
void f407_usart_set_baudrate(f407_usart_handle_t *handle, uint32_t baudrate)
{
    if (handle == NULL || !handle->initialized)
        return;

    USART_TypeDef *usart = handle->instance;

    /* 禁用USART */
    usart->CR1 &= ~USART_CR1_UE;

    /* 设置波特率 */
    uint32_t clk = usart_clk_table[handle->config.usart];
    usart->BRR = clk / baudrate;

    /* 使能USART */
    usart->CR1 |= USART_CR1_UE;

    handle->config.baudrate = baudrate;
}

/**
 * @brief 检查发送是否完成
 */
bool f407_usart_tx_complete(f407_usart_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return false;
    return (handle->instance->SR & USART_SR_TC) ? true : false;
}

/**
 * @brief 检查是否有数据可接收
 */
bool f407_usart_rx_ready(f407_usart_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return false;
    return (handle->instance->SR & USART_SR_RXNE) ? true : false;
}

/**
 * @brief 等待发送完成
 */
int f407_usart_wait_tx_complete(f407_usart_handle_t *handle, uint32_t timeout)
{
    if (handle == NULL || !handle->initialized)
        return -1;

    while (!(handle->instance->SR & USART_SR_TC))
    {
        if (--timeout == 0)
            return -1;
    }

    return 0;
}

/**
 * @brief 清除错误标志
 */
void f407_usart_clear_errors(f407_usart_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;

    /* 读SR和DR清除错误标志 */
    (void)handle->instance->SR;
    (void)handle->instance->DR;
}
