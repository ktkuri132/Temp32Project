/**
 * @file vet6_usart.c
 * @brief STM32F407VET6 USART板级驱动实现
 * @details 提供USART1-USART6的初始化和操作API
 */

#include "vet6_usart.h"
#include <stdio.h>
#include <string.h>

/*===========================================================================*/
/*                              内部变量                                      */
/*===========================================================================*/

/* USART外设基地址表 */
static USART_TypeDef *const usart_base_table[VET6_USART_MAX] = {
    USART1, USART2, USART3, UART4, UART5, USART6};

/* USART中断号表 */
static const IRQn_Type usart_irq_table[VET6_USART_MAX] = {
    USART1_IRQn, USART2_IRQn, USART3_IRQn,
    UART4_IRQn, UART5_IRQn, USART6_IRQn};

/* USART时钟表 */
static const uint32_t usart_clk_table[VET6_USART_MAX] = {
    VET6_USART1_CLK, VET6_USART2_CLK, VET6_USART3_CLK,
    VET6_UART4_CLK, VET6_UART5_CLK, VET6_USART6_CLK};

/*===========================================================================*/
/*                              时钟控制                                      */
/*===========================================================================*/

/**
 * @brief 使能USART时钟
 */
void vet6_usart_clk_enable(vet6_usart_t usart)
{
    switch (usart)
    {
    case VET6_USART1:
        RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
        break;
    case VET6_USART2:
        RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
        break;
    case VET6_USART3:
        RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
        break;
    case VET6_UART4:
        RCC->APB1ENR |= RCC_APB1ENR_UART4EN;
        break;
    case VET6_UART5:
        RCC->APB1ENR |= RCC_APB1ENR_UART5EN;
        break;
    case VET6_USART6:
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
void vet6_usart_clk_disable(vet6_usart_t usart)
{
    switch (usart)
    {
    case VET6_USART1:
        RCC->APB2ENR &= ~RCC_APB2ENR_USART1EN;
        break;
    case VET6_USART2:
        RCC->APB1ENR &= ~RCC_APB1ENR_USART2EN;
        break;
    case VET6_USART3:
        RCC->APB1ENR &= ~RCC_APB1ENR_USART3EN;
        break;
    case VET6_UART4:
        RCC->APB1ENR &= ~RCC_APB1ENR_UART4EN;
        break;
    case VET6_UART5:
        RCC->APB1ENR &= ~RCC_APB1ENR_UART5EN;
        break;
    case VET6_USART6:
        RCC->APB2ENR &= ~RCC_APB2ENR_USART6EN;
        break;
    default:
        break;
    }
}

/**
 * @brief 获取USART外设基地址
 */
USART_TypeDef *vet6_usart_get_base(vet6_usart_t usart)
{
    if (usart >= VET6_USART_MAX)
        return NULL;
    return usart_base_table[usart];
}

/**
 * @brief 获取USART时钟频率
 */
uint32_t vet6_usart_get_clk(vet6_usart_t usart)
{
    if (usart >= VET6_USART_MAX)
        return 0;
    return usart_clk_table[usart];
}

/*===========================================================================*/
/*                              引脚配置                                      */
/*===========================================================================*/

/**
 * @brief 初始化USART引脚
 */
int vet6_usart_pins_init(vet6_usart_pinmap_t pinmap)
{
    switch (pinmap)
    {
    case VET6_USART1_PINS_PA9_PA10:
        /* USART1: PA9(TX), PA10(RX) - AF7 */
        vet6_gpio_config_af(VET6_PA9, VET6_GPIO_AF7_USART1_3);
        vet6_gpio_config_af(VET6_PA10, VET6_GPIO_AF7_USART1_3);
        break;

    case VET6_USART1_PINS_PB6_PB7:
        /* USART1: PB6(TX), PB7(RX) - AF7 */
        vet6_gpio_config_af(VET6_PB6, VET6_GPIO_AF7_USART1_3);
        vet6_gpio_config_af(VET6_PB7, VET6_GPIO_AF7_USART1_3);
        break;

    case VET6_USART2_PINS_PA2_PA3:
        /* USART2: PA2(TX), PA3(RX) - AF7 */
        vet6_gpio_config_af(VET6_PA2, VET6_GPIO_AF7_USART1_3);
        vet6_gpio_config_af(VET6_PA3, VET6_GPIO_AF7_USART1_3);
        break;

    case VET6_USART2_PINS_PD5_PD6:
        /* USART2: PD5(TX), PD6(RX) - AF7 */
        vet6_gpio_config_af(VET6_PD5, VET6_GPIO_AF7_USART1_3);
        vet6_gpio_config_af(VET6_PD6, VET6_GPIO_AF7_USART1_3);
        break;

    case VET6_USART3_PINS_PB10_PB11:
        /* USART3: PB10(TX), PB11(RX) - AF7 */
        vet6_gpio_config_af(VET6_PB10, VET6_GPIO_AF7_USART1_3);
        vet6_gpio_config_af(VET6_PB11, VET6_GPIO_AF7_USART1_3);
        break;

    case VET6_USART3_PINS_PC10_PC11:
        /* USART3: PC10(TX), PC11(RX) - AF7 */
        vet6_gpio_config_af(VET6_PC10, VET6_GPIO_AF7_USART1_3);
        vet6_gpio_config_af(VET6_PC11, VET6_GPIO_AF7_USART1_3);
        break;

    case VET6_USART3_PINS_PD8_PD9:
        /* USART3: PD8(TX), PD9(RX) - AF7 */
        vet6_gpio_config_af(VET6_PD8, VET6_GPIO_AF7_USART1_3);
        vet6_gpio_config_af(VET6_PD9, VET6_GPIO_AF7_USART1_3);
        break;

    case VET6_UART4_PINS_PA0_PA1:
        /* UART4: PA0(TX), PA1(RX) - AF8 */
        vet6_gpio_config_af(VET6_PA0, VET6_GPIO_AF8_USART4_6);
        vet6_gpio_config_af(VET6_PA1, VET6_GPIO_AF8_USART4_6);
        break;

    case VET6_UART4_PINS_PC10_PC11:
        /* UART4: PC10(TX), PC11(RX) - AF8 */
        vet6_gpio_config_af(VET6_PC10, VET6_GPIO_AF8_USART4_6);
        vet6_gpio_config_af(VET6_PC11, VET6_GPIO_AF8_USART4_6);
        break;

    case VET6_UART5_PINS_PC12_PD2:
        /* UART5: PC12(TX), PD2(RX) - AF8 */
        vet6_gpio_config_af(VET6_PC12, VET6_GPIO_AF8_USART4_6);
        vet6_gpio_config_af(VET6_PD2, VET6_GPIO_AF8_USART4_6);
        break;

    case VET6_USART6_PINS_PC6_PC7:
        /* USART6: PC6(TX), PC7(RX) - AF8 */
        vet6_gpio_config_af(VET6_PC6, VET6_GPIO_AF8_USART4_6);
        vet6_gpio_config_af(VET6_PC7, VET6_GPIO_AF8_USART4_6);
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
int vet6_usart_init(vet6_usart_handle_t *handle, const vet6_usart_config_t *config)
{
    if (handle == NULL || config == NULL)
        return -1;
    if (config->usart >= VET6_USART_MAX)
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
    vet6_usart_clk_enable(config->usart);

    /* 初始化引脚 */
    if (vet6_usart_pins_init(config->pinmap) != 0)
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
    if (config->wordlen == VET6_USART_WORDLEN_9B)
    {
        cr1 |= USART_CR1_M;
    }

    /* 校验位 */
    if (config->parity != VET6_USART_PARITY_NONE)
    {
        cr1 |= USART_CR1_PCE;
        if (config->parity == VET6_USART_PARITY_ODD)
        {
            cr1 |= USART_CR1_PS;
        }
    }

    /* 发送/接收使能 */
    if (config->mode & VET6_USART_MODE_TX)
    {
        cr1 |= USART_CR1_TE;
    }
    if (config->mode & VET6_USART_MODE_RX)
    {
        cr1 |= USART_CR1_RE;
    }

    usart->CR1 = cr1;

    /* 配置CR2 - 停止位 */
    usart->CR2 &= ~USART_CR2_STOP;
    usart->CR2 |= ((uint32_t)config->stopbits << 12);

    /* 配置CR3 - 硬件流控 */
    usart->CR3 &= ~(USART_CR3_RTSE | USART_CR3_CTSE);
    if (config->hwflow & VET6_USART_HWFLOW_RTS)
    {
        usart->CR3 |= USART_CR3_RTSE;
    }
    if (config->hwflow & VET6_USART_HWFLOW_CTS)
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
int vet6_usart_init_simple(vet6_usart_handle_t *handle, vet6_usart_t usart,
                           uint32_t baudrate, vet6_usart_pinmap_t pinmap)
{
    vet6_usart_config_t config = {
        .usart = usart,
        .baudrate = baudrate,
        .wordlen = VET6_USART_WORDLEN_8B,
        .stopbits = VET6_USART_STOPBITS_1,
        .parity = VET6_USART_PARITY_NONE,
        .mode = VET6_USART_MODE_TX_RX,
        .hwflow = VET6_USART_HWFLOW_NONE,
        .pinmap = pinmap};

    return vet6_usart_init(handle, &config);
}

/**
 * @brief 反初始化USART
 */
void vet6_usart_deinit(vet6_usart_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;

    /* 禁用USART */
    handle->instance->CR1 &= ~USART_CR1_UE;

    /* 禁用中断 */
    NVIC_DisableIRQ(usart_irq_table[handle->config.usart]);

    /* 禁用时钟 */
    vet6_usart_clk_disable(handle->config.usart);

    handle->initialized = false;
}

/*===========================================================================*/
/*                              基本操作                                      */
/*===========================================================================*/

/**
 * @brief 使能USART
 */
void vet6_usart_enable(vet6_usart_handle_t *handle)
{
    if (handle == NULL)
        return;
    handle->instance->CR1 |= USART_CR1_UE;
}

/**
 * @brief 禁用USART
 */
void vet6_usart_disable(vet6_usart_handle_t *handle)
{
    if (handle == NULL)
        return;
    handle->instance->CR1 &= ~USART_CR1_UE;
}

/**
 * @brief 发送单字节
 */
int vet6_usart_send_byte(vet6_usart_handle_t *handle, uint8_t data)
{
    if (handle == NULL || !handle->initialized)
        return -1;

    USART_TypeDef *usart = handle->instance;
    uint32_t timeout = VET6_USART_TIMEOUT;

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
int vet6_usart_receive_byte(vet6_usart_handle_t *handle, uint8_t *data, uint32_t timeout)
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
int vet6_usart_send(vet6_usart_handle_t *handle, const uint8_t *data, uint16_t len)
{
    if (handle == NULL || data == NULL || !handle->initialized)
        return -1;

    for (uint16_t i = 0; i < len; i++)
    {
        if (vet6_usart_send_byte(handle, data[i]) != 0)
        {
            return -1;
        }
    }

    return 0;
}

/**
 * @brief 发送字符串
 */
int vet6_usart_send_string(vet6_usart_handle_t *handle, const char *str)
{
    if (handle == NULL || str == NULL || !handle->initialized)
        return -1;

    while (*str)
    {
        if (vet6_usart_send_byte(handle, *str++) != 0)
        {
            return -1;
        }
    }

    return 0;
}

/**
 * @brief 接收数据到缓冲区(阻塞)
 */
int vet6_usart_receive(vet6_usart_handle_t *handle, uint8_t *data, uint16_t len, uint32_t timeout)
{
    if (handle == NULL || data == NULL || !handle->initialized)
        return 0;

    uint16_t received = 0;

    while (received < len)
    {
        if (vet6_usart_receive_byte(handle, &data[received], timeout) != 0)
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
int vet6_usart_printf(vet6_usart_handle_t *handle, const char *format, ...)
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
        vet6_usart_send(handle, (uint8_t *)buffer, len);
    }

    return len;
}

/*===========================================================================*/
/*                              中断相关                                      */
/*===========================================================================*/

/**
 * @brief 使能USART接收中断
 */
void vet6_usart_enable_rx_irq(vet6_usart_handle_t *handle, uint8_t priority)
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
void vet6_usart_disable_rx_irq(vet6_usart_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;

    handle->instance->CR1 &= ~USART_CR1_RXNEIE;
    NVIC_DisableIRQ(usart_irq_table[handle->config.usart]);
}

/**
 * @brief 设置接收回调函数
 */
void vet6_usart_set_rx_callback(vet6_usart_handle_t *handle, vet6_usart_rx_callback_t callback)
{
    if (handle == NULL)
        return;
    handle->rx_callback = callback;
}

/**
 * @brief 设置接收缓冲区
 */
void vet6_usart_set_rx_buffer(vet6_usart_handle_t *handle, uint8_t *buffer, uint16_t size)
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
int vet6_usart_read_buffer(vet6_usart_handle_t *handle, uint8_t *data, uint16_t len)
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
uint16_t vet6_usart_available(vet6_usart_handle_t *handle)
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
void vet6_usart_flush_rx(vet6_usart_handle_t *handle)
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
void vet6_usart_irq_handler(vet6_usart_handle_t *handle)
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
void vet6_usart_set_baudrate(vet6_usart_handle_t *handle, uint32_t baudrate)
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
bool vet6_usart_tx_complete(vet6_usart_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return false;
    return (handle->instance->SR & USART_SR_TC) ? true : false;
}

/**
 * @brief 检查是否有数据可接收
 */
bool vet6_usart_rx_ready(vet6_usart_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return false;
    return (handle->instance->SR & USART_SR_RXNE) ? true : false;
}

/**
 * @brief 等待发送完成
 */
int vet6_usart_wait_tx_complete(vet6_usart_handle_t *handle, uint32_t timeout)
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
void vet6_usart_clear_errors(vet6_usart_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;

    /* 读SR和DR清除错误标志 */
    (void)handle->instance->SR;
    (void)handle->instance->DR;
}
