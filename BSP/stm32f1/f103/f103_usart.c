/**
 * @file f103_usart.c
 * @brief STM32F103 系列 USART 驱动实现
 */

#include "f103_usart.h"
#include "f103_gpio.h"

/*===========================================================================*/
/*                            内部数据表                                      */
/*===========================================================================*/

static USART_TypeDef *const usart_base_table[F103_USART_MAX] = {
    USART1, USART2, USART3};

/*===========================================================================*/
/*                            时钟和GPIO配置                                  */
/*===========================================================================*/

static void f103_usart_clk_enable(f103_usart_port_t port)
{
    switch (port)
    {
    case F103_USART1:
        RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
        break;
    case F103_USART2:
        RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
        break;
    case F103_USART3:
        RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
        break;
    default:
        break;
    }
}

static void f103_usart_gpio_init(f103_usart_port_t port)
{
    switch (port)
    {
    case F103_USART1:
        /* PA9: TX, PA10: RX */
        f103_gpio_init_quick(F103_GPIOA, F103_PIN_9, F103_GPIO_MODE_AF_PP, F103_GPIO_SPEED_50MHZ);
        f103_gpio_init_quick(F103_GPIOA, F103_PIN_10, F103_GPIO_MODE_IN_FLOATING, F103_GPIO_SPEED_2MHZ);
        break;
    case F103_USART2:
        /* PA2: TX, PA3: RX */
        f103_gpio_init_quick(F103_GPIOA, F103_PIN_2, F103_GPIO_MODE_AF_PP, F103_GPIO_SPEED_50MHZ);
        f103_gpio_init_quick(F103_GPIOA, F103_PIN_3, F103_GPIO_MODE_IN_FLOATING, F103_GPIO_SPEED_2MHZ);
        break;
    case F103_USART3:
        /* PB10: TX, PB11: RX */
        f103_gpio_init_quick(F103_GPIOB, F103_PIN_10, F103_GPIO_MODE_AF_PP, F103_GPIO_SPEED_50MHZ);
        f103_gpio_init_quick(F103_GPIOB, F103_PIN_11, F103_GPIO_MODE_IN_FLOATING, F103_GPIO_SPEED_2MHZ);
        break;
    default:
        break;
    }
}

/*===========================================================================*/
/*                            API实现                                         */
/*===========================================================================*/

int f103_usart_init(const f103_usart_config_t *config)
{
    if (config == NULL || config->port >= F103_USART_MAX)
        return -1;

    USART_TypeDef *usart = usart_base_table[config->port];

    /* 使能时钟 */
    f103_usart_clk_enable(config->port);

    /* 配置GPIO */
    f103_usart_gpio_init(config->port);

    /* 获取时钟频率 */
    uint32_t pclk;
    if (config->port == F103_USART1)
    {
        pclk = 72000000; /* USART1在APB2上，假设72MHz */
    }
    else
    {
        pclk = 36000000; /* USART2/3在APB1上，假设36MHz */
    }

    /* 计算波特率 */
    usart->BRR = pclk / config->baudrate;

    /* 配置控制寄存器 */
    usart->CR1 = 0;

    /* 数据位 */
    if (config->word_length == 9)
    {
        usart->CR1 |= USART_CR1_M;
    }

    /* 校验位 */
    if (config->parity == 1)
    {
        usart->CR1 |= USART_CR1_PCE | USART_CR1_PS; /* 奇校验 */
    }
    else if (config->parity == 2)
    {
        usart->CR1 |= USART_CR1_PCE; /* 偶校验 */
    }

    /* 停止位 */
    usart->CR2 = 0;
    if (config->stop_bits == 2)
    {
        usart->CR2 |= USART_CR2_STOP_1;
    }

    /* 使能发送和接收 */
    usart->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;

    return 0;
}

int f103_usart_init_quick(f103_usart_port_t port, uint32_t baudrate)
{
    f103_usart_config_t config = {
        .port = port,
        .baudrate = baudrate,
        .word_length = 8,
        .stop_bits = 1,
        .parity = 0};
    return f103_usart_init(&config);
}

void f103_usart_send_char(f103_usart_port_t port, uint8_t ch)
{
    if (port >= F103_USART_MAX)
        return;

    USART_TypeDef *usart = usart_base_table[port];

    while (!(usart->SR & USART_SR_TXE))
        ;
    usart->DR = ch;
}

void f103_usart_send_string(f103_usart_port_t port, const char *str)
{
    while (*str)
    {
        f103_usart_send_char(port, *str++);
    }
}

void f103_usart_send_data(f103_usart_port_t port, const uint8_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
    {
        f103_usart_send_char(port, data[i]);
    }
}

uint8_t f103_usart_recv_char(f103_usart_port_t port)
{
    if (port >= F103_USART_MAX)
        return 0;

    USART_TypeDef *usart = usart_base_table[port];

    while (!(usart->SR & USART_SR_RXNE))
        ;
    return (uint8_t)usart->DR;
}

bool f103_usart_available(f103_usart_port_t port)
{
    if (port >= F103_USART_MAX)
        return false;

    USART_TypeDef *usart = usart_base_table[port];
    return (usart->SR & USART_SR_RXNE) != 0;
}
