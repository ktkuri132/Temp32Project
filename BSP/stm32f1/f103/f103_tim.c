/**
 * @file f103_tim.c
 * @brief STM32F103 系列定时器驱动实现
 */

#include "f103_tim.h"
#include "f103_gpio.h"

static TIM_TypeDef *const tim_base_table[F103_TIM_MAX] = {
    TIM1, TIM2, TIM3, TIM4};

static const IRQn_Type tim_irq_table[F103_TIM_MAX] = {
    TIM1_UP_IRQn, TIM2_IRQn, TIM3_IRQn, TIM4_IRQn};

static void f103_tim_clk_enable(f103_tim_port_t port)
{
    switch (port)
    {
    case F103_TIM1:
        RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
        break;
    case F103_TIM2:
        RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
        break;
    case F103_TIM3:
        RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
        break;
    case F103_TIM4:
        RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
        break;
    default:
        break;
    }
}

int f103_tim_init(const f103_tim_config_t *config)
{
    if (config == NULL || config->port >= F103_TIM_MAX)
        return -1;

    TIM_TypeDef *tim = tim_base_table[config->port];

    f103_tim_clk_enable(config->port);

    /* 禁用定时器 */
    tim->CR1 = 0;

    /* 设置预分频器 */
    tim->PSC = config->prescaler - 1;

    /* 设置自动重载值 */
    tim->ARR = config->period - 1;

    /* 设置计数方向 */
    if (config->dir == F103_TIM_COUNT_DOWN)
    {
        tim->CR1 |= TIM_CR1_DIR;
    }

    /* 使能更新中断 */
    if (config->enable_irq)
    {
        tim->DIER |= TIM_DIER_UIE;
        NVIC_EnableIRQ(tim_irq_table[config->port]);
    }

    /* 产生更新事件加载预分频器 */
    tim->EGR |= TIM_EGR_UG;

    /* 清除更新标志 */
    tim->SR &= ~TIM_SR_UIF;

    return 0;
}

int f103_tim_init_1ms(f103_tim_port_t port, bool enable_irq)
{
    /* 假设系统时钟 72MHz */
    f103_tim_config_t config = {
        .port = port,
        .mode = F103_TIM_MODE_TIMER,
        .prescaler = 72, /* 72MHz/72 = 1MHz */
        .period = 1000,  /* 1MHz/1000 = 1kHz = 1ms */
        .dir = F103_TIM_COUNT_UP,
        .enable_irq = enable_irq};
    return f103_tim_init(&config);
}

void f103_tim_start(f103_tim_port_t port)
{
    if (port < F103_TIM_MAX)
    {
        tim_base_table[port]->CR1 |= TIM_CR1_CEN;
    }
}

void f103_tim_stop(f103_tim_port_t port)
{
    if (port < F103_TIM_MAX)
    {
        tim_base_table[port]->CR1 &= ~TIM_CR1_CEN;
    }
}

uint32_t f103_tim_get_count(f103_tim_port_t port)
{
    if (port < F103_TIM_MAX)
    {
        return tim_base_table[port]->CNT;
    }
    return 0;
}

void f103_tim_set_count(f103_tim_port_t port, uint32_t count)
{
    if (port < F103_TIM_MAX)
    {
        tim_base_table[port]->CNT = count;
    }
}

void f103_tim_clear_irq(f103_tim_port_t port)
{
    if (port < F103_TIM_MAX)
    {
        tim_base_table[port]->SR &= ~TIM_SR_UIF;
    }
}

/* PWM GPIO初始化 */
static void f103_pwm_gpio_init(f103_tim_port_t port, f103_tim_channel_t channel)
{
    switch (port)
    {
    case F103_TIM1:
        /* TIM1: PA8-PA11 */
        f103_gpio_init_quick(F103_GPIOA, F103_PIN_8 + channel, F103_GPIO_MODE_AF_PP, F103_GPIO_SPEED_50MHZ);
        break;
    case F103_TIM2:
        /* TIM2: PA0-PA3 */
        f103_gpio_init_quick(F103_GPIOA, channel, F103_GPIO_MODE_AF_PP, F103_GPIO_SPEED_50MHZ);
        break;
    case F103_TIM3:
        /* TIM3: PA6-PA7, PB0-PB1 */
        if (channel <= F103_TIM_CH2)
        {
            f103_gpio_init_quick(F103_GPIOA, F103_PIN_6 + channel, F103_GPIO_MODE_AF_PP, F103_GPIO_SPEED_50MHZ);
        }
        else
        {
            f103_gpio_init_quick(F103_GPIOB, channel - 2, F103_GPIO_MODE_AF_PP, F103_GPIO_SPEED_50MHZ);
        }
        break;
    case F103_TIM4:
        /* TIM4: PB6-PB9 */
        f103_gpio_init_quick(F103_GPIOB, F103_PIN_6 + channel, F103_GPIO_MODE_AF_PP, F103_GPIO_SPEED_50MHZ);
        break;
    default:
        break;
    }
}

int f103_pwm_init(const f103_pwm_config_t *config)
{
    if (config == NULL || config->port >= F103_TIM_MAX || config->channel >= F103_TIM_CH_MAX)
        return -1;

    TIM_TypeDef *tim = tim_base_table[config->port];

    f103_tim_clk_enable(config->port);
    f103_pwm_gpio_init(config->port, config->channel);

    /* 基本定时器配置 */
    tim->PSC = config->prescaler - 1;
    tim->ARR = config->period - 1;
    tim->CR1 &= ~TIM_CR1_DIR;

    /* 配置PWM模式1 */
    uint32_t ccmr_mode = 0x60; /* PWM模式1 */
    uint32_t ccer_mask = 0;
    volatile uint32_t *ccr_reg;

    switch (config->channel)
    {
    case F103_TIM_CH1:
        tim->CCMR1 = (tim->CCMR1 & 0xFF00) | (ccmr_mode << 0);
        ccer_mask = TIM_CCER_CC1E;
        ccr_reg = &tim->CCR1;
        break;
    case F103_TIM_CH2:
        tim->CCMR1 = (tim->CCMR1 & 0x00FF) | (ccmr_mode << 8);
        ccer_mask = TIM_CCER_CC2E;
        ccr_reg = &tim->CCR2;
        break;
    case F103_TIM_CH3:
        tim->CCMR2 = (tim->CCMR2 & 0xFF00) | (ccmr_mode << 0);
        ccer_mask = TIM_CCER_CC3E;
        ccr_reg = &tim->CCR3;
        break;
    case F103_TIM_CH4:
        tim->CCMR2 = (tim->CCMR2 & 0x00FF) | (ccmr_mode << 8);
        ccer_mask = TIM_CCER_CC4E;
        ccr_reg = &tim->CCR4;
        break;
    default:
        return -1;
    }

    /* 设置脉冲宽度 */
    *ccr_reg = config->pulse;

    /* 使能通道输出 */
    tim->CCER |= ccer_mask;

    /* TIM1高级定时器需要使能MOE */
    if (config->port == F103_TIM1)
    {
        tim->BDTR |= TIM_BDTR_MOE;
    }

    /* 产生更新事件 */
    tim->EGR |= TIM_EGR_UG;

    return 0;
}

int f103_pwm_init_quick(f103_tim_port_t port, f103_tim_channel_t channel,
                        uint32_t freq, uint8_t duty)
{
    if (freq == 0)
        return -1;

    /* 假设系统时钟72MHz */
    uint32_t pclk = 72000000;
    uint32_t prescaler = 72;
    uint32_t period = pclk / prescaler / freq;

    if (period > 65535)
    {
        prescaler = 7200;
        period = pclk / prescaler / freq;
    }

    f103_pwm_config_t config = {
        .port = port,
        .channel = channel,
        .prescaler = prescaler,
        .period = period,
        .pulse = period * duty / 100,
        .output_polarity = true};

    return f103_pwm_init(&config);
}

void f103_pwm_set_duty(f103_tim_port_t port, f103_tim_channel_t channel, uint8_t duty)
{
    if (port >= F103_TIM_MAX || channel >= F103_TIM_CH_MAX)
        return;

    TIM_TypeDef *tim = tim_base_table[port];
    uint32_t pulse = (tim->ARR + 1) * duty / 100;

    f103_pwm_set_pulse(port, channel, pulse);
}

void f103_pwm_set_pulse(f103_tim_port_t port, f103_tim_channel_t channel, uint32_t pulse)
{
    if (port >= F103_TIM_MAX || channel >= F103_TIM_CH_MAX)
        return;

    TIM_TypeDef *tim = tim_base_table[port];

    switch (channel)
    {
    case F103_TIM_CH1:
        tim->CCR1 = pulse;
        break;
    case F103_TIM_CH2:
        tim->CCR2 = pulse;
        break;
    case F103_TIM_CH3:
        tim->CCR3 = pulse;
        break;
    case F103_TIM_CH4:
        tim->CCR4 = pulse;
        break;
    default:
        break;
    }
}

void f103_pwm_start(f103_tim_port_t port, f103_tim_channel_t channel)
{
    if (port >= F103_TIM_MAX)
        return;

    TIM_TypeDef *tim = tim_base_table[port];

    /* 使能通道 */
    tim->CCER |= (1 << (channel * 4));

    /* 使能定时器 */
    tim->CR1 |= TIM_CR1_CEN;
}

void f103_pwm_stop(f103_tim_port_t port, f103_tim_channel_t channel)
{
    if (port >= F103_TIM_MAX)
        return;

    TIM_TypeDef *tim = tim_base_table[port];

    /* 禁用通道 */
    tim->CCER &= ~(1 << (channel * 4));
}

/* 编码器GPIO初始化 */
static void f103_encoder_gpio_init(f103_tim_port_t port)
{
    switch (port)
    {
    case F103_TIM2:
        /* TIM2 CH1/CH2: PA0/PA1 */
        f103_gpio_init_quick(F103_GPIOA, F103_PIN_0, F103_GPIO_MODE_IN_FLOATING, F103_GPIO_SPEED_50MHZ);
        f103_gpio_init_quick(F103_GPIOA, F103_PIN_1, F103_GPIO_MODE_IN_FLOATING, F103_GPIO_SPEED_50MHZ);
        break;
    case F103_TIM3:
        /* TIM3 CH1/CH2: PA6/PA7 */
        f103_gpio_init_quick(F103_GPIOA, F103_PIN_6, F103_GPIO_MODE_IN_FLOATING, F103_GPIO_SPEED_50MHZ);
        f103_gpio_init_quick(F103_GPIOA, F103_PIN_7, F103_GPIO_MODE_IN_FLOATING, F103_GPIO_SPEED_50MHZ);
        break;
    case F103_TIM4:
        /* TIM4 CH1/CH2: PB6/PB7 */
        f103_gpio_init_quick(F103_GPIOB, F103_PIN_6, F103_GPIO_MODE_IN_FLOATING, F103_GPIO_SPEED_50MHZ);
        f103_gpio_init_quick(F103_GPIOB, F103_PIN_7, F103_GPIO_MODE_IN_FLOATING, F103_GPIO_SPEED_50MHZ);
        break;
    default:
        break;
    }
}

int f103_encoder_init(f103_tim_port_t port)
{
    if (port >= F103_TIM_MAX || port == F103_TIM1)
        return -1;

    TIM_TypeDef *tim = tim_base_table[port];

    f103_tim_clk_enable(port);
    f103_encoder_gpio_init(port);

    /* 配置编码器模式 */
    tim->SMCR = (tim->SMCR & ~TIM_SMCR_SMS) | (0x03 << 0); /* 编码器模式3 */

    /* 配置输入捕获 */
    tim->CCMR1 = (1 << 0) | (1 << 8); /* CC1S=01, CC2S=01 */

    /* 不反相 */
    tim->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC2P);

    /* 设置自动重载值为最大 */
    tim->ARR = 0xFFFF;

    /* 复位计数器 */
    tim->CNT = 0x8000; /* 从中间开始，方便检测正反转 */

    /* 使能定时器 */
    tim->CR1 |= TIM_CR1_CEN;

    return 0;
}

int32_t f103_encoder_get_count(f103_tim_port_t port)
{
    if (port >= F103_TIM_MAX)
        return 0;

    int32_t count = (int32_t)tim_base_table[port]->CNT - 0x8000;
    return count;
}

void f103_encoder_reset(f103_tim_port_t port)
{
    if (port < F103_TIM_MAX)
    {
        tim_base_table[port]->CNT = 0x8000;
    }
}
