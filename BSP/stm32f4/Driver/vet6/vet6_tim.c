/**
 * @file vet6_tim.c
 * @brief STM32F407VET6 定时器板级驱动实现
 * @details 提供TIM1-TIM14的初始化和操作API
 */

#include "vet6_tim.h"

/*===========================================================================*/
/*                              内部变量                                      */
/*===========================================================================*/

/* 定时器外设基地址表 */
static TIM_TypeDef *const tim_base_table[VET6_TIM_MAX] = {
    TIM1, TIM2, TIM3, TIM4, TIM5, TIM6, TIM7, TIM8,
    TIM9, TIM10, TIM11, TIM12, TIM13, TIM14};

/* 定时器中断号表 */
static const IRQn_Type tim_irq_table[VET6_TIM_MAX] = {
    TIM1_UP_TIM10_IRQn,      // TIM1
    TIM2_IRQn,               // TIM2
    TIM3_IRQn,               // TIM3
    TIM4_IRQn,               // TIM4
    TIM5_IRQn,               // TIM5
    TIM6_DAC_IRQn,           // TIM6
    TIM7_IRQn,               // TIM7
    TIM8_UP_TIM13_IRQn,      // TIM8
    TIM1_BRK_TIM9_IRQn,      // TIM9
    TIM1_UP_TIM10_IRQn,      // TIM10
    TIM1_TRG_COM_TIM11_IRQn, // TIM11
    TIM8_BRK_TIM12_IRQn,     // TIM12
    TIM8_UP_TIM13_IRQn,      // TIM13
    TIM8_TRG_COM_TIM14_IRQn  // TIM14
};

/*===========================================================================*/
/*                              时钟控制                                      */
/*===========================================================================*/

/**
 * @brief 使能定时器时钟
 */
void vet6_tim_clk_enable(vet6_tim_t tim)
{
    switch (tim)
    {
    case VET6_TIM1:
        RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
        break;
    case VET6_TIM2:
        RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
        break;
    case VET6_TIM3:
        RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
        break;
    case VET6_TIM4:
        RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
        break;
    case VET6_TIM5:
        RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;
        break;
    case VET6_TIM6:
        RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
        break;
    case VET6_TIM7:
        RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
        break;
    case VET6_TIM8:
        RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;
        break;
    case VET6_TIM9:
        RCC->APB2ENR |= RCC_APB2ENR_TIM9EN;
        break;
    case VET6_TIM10:
        RCC->APB2ENR |= RCC_APB2ENR_TIM10EN;
        break;
    case VET6_TIM11:
        RCC->APB2ENR |= RCC_APB2ENR_TIM11EN;
        break;
    case VET6_TIM12:
        RCC->APB1ENR |= RCC_APB1ENR_TIM12EN;
        break;
    case VET6_TIM13:
        RCC->APB1ENR |= RCC_APB1ENR_TIM13EN;
        break;
    case VET6_TIM14:
        RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
        break;
    default:
        break;
    }
    __DSB();
}

/**
 * @brief 禁用定时器时钟
 */
void vet6_tim_clk_disable(vet6_tim_t tim)
{
    switch (tim)
    {
    case VET6_TIM1:
        RCC->APB2ENR &= ~RCC_APB2ENR_TIM1EN;
        break;
    case VET6_TIM2:
        RCC->APB1ENR &= ~RCC_APB1ENR_TIM2EN;
        break;
    case VET6_TIM3:
        RCC->APB1ENR &= ~RCC_APB1ENR_TIM3EN;
        break;
    case VET6_TIM4:
        RCC->APB1ENR &= ~RCC_APB1ENR_TIM4EN;
        break;
    case VET6_TIM5:
        RCC->APB1ENR &= ~RCC_APB1ENR_TIM5EN;
        break;
    case VET6_TIM6:
        RCC->APB1ENR &= ~RCC_APB1ENR_TIM6EN;
        break;
    case VET6_TIM7:
        RCC->APB1ENR &= ~RCC_APB1ENR_TIM7EN;
        break;
    case VET6_TIM8:
        RCC->APB2ENR &= ~RCC_APB2ENR_TIM8EN;
        break;
    case VET6_TIM9:
        RCC->APB2ENR &= ~RCC_APB2ENR_TIM9EN;
        break;
    case VET6_TIM10:
        RCC->APB2ENR &= ~RCC_APB2ENR_TIM10EN;
        break;
    case VET6_TIM11:
        RCC->APB2ENR &= ~RCC_APB2ENR_TIM11EN;
        break;
    case VET6_TIM12:
        RCC->APB1ENR &= ~RCC_APB1ENR_TIM12EN;
        break;
    case VET6_TIM13:
        RCC->APB1ENR &= ~RCC_APB1ENR_TIM13EN;
        break;
    case VET6_TIM14:
        RCC->APB1ENR &= ~RCC_APB1ENR_TIM14EN;
        break;
    default:
        break;
    }
}

/**
 * @brief 获取定时器外设基地址
 */
TIM_TypeDef *vet6_tim_get_base(vet6_tim_t tim)
{
    if (tim >= VET6_TIM_MAX)
        return NULL;
    return tim_base_table[tim];
}

/**
 * @brief 获取定时器时钟频率
 */
uint32_t vet6_tim_get_clk(vet6_tim_t tim)
{
    /* APB2定时器: TIM1, TIM8, TIM9, TIM10, TIM11 */
    if (tim == VET6_TIM1 || tim == VET6_TIM8 ||
        tim == VET6_TIM9 || tim == VET6_TIM10 || tim == VET6_TIM11)
    {
        return VET6_TIM_APB2_CLK;
    }
    /* APB1定时器: TIM2-7, TIM12-14 */
    return VET6_TIM_APB1_CLK;
}

/**
 * @brief 检查定时器是否支持32位计数
 */
bool vet6_tim_is_32bit(vet6_tim_t tim)
{
    return (tim == VET6_TIM2 || tim == VET6_TIM5);
}

/**
 * @brief 检查定时器是否为高级定时器
 */
bool vet6_tim_is_advanced(vet6_tim_t tim)
{
    return (tim == VET6_TIM1 || tim == VET6_TIM8);
}

/*===========================================================================*/
/*                              基本定时功能                                  */
/*===========================================================================*/

/**
 * @brief 初始化定时器基本配置
 */
int vet6_tim_base_init(const vet6_tim_base_config_t *config)
{
    if (config == NULL || config->tim >= VET6_TIM_MAX)
        return -1;

    TIM_TypeDef *tim = tim_base_table[config->tim];

    /* 使能时钟 */
    vet6_tim_clk_enable(config->tim);

    /* 停止定时器 */
    tim->CR1 &= ~TIM_CR1_CEN;

    /* 设置预分频 */
    tim->PSC = config->prescaler;

    /* 设置自动重载值 */
    tim->ARR = config->period;

    /* 配置计数方向 */
    if (config->dir == VET6_TIM_DIR_DOWN)
    {
        tim->CR1 |= TIM_CR1_DIR;
    }
    else
    {
        tim->CR1 &= ~TIM_CR1_DIR;
    }

    /* 配置自动重载预装载 */
    if (config->auto_reload)
    {
        tim->CR1 |= TIM_CR1_ARPE;
    }
    else
    {
        tim->CR1 &= ~TIM_CR1_ARPE;
    }

    /* 触发更新事件以加载预分频值 */
    tim->EGR = TIM_EGR_UG;

    /* 清除更新标志 */
    tim->SR &= ~TIM_SR_UIF;

    return 0;
}

/**
 * @brief 快速初始化定时器为微秒级定时
 */
int vet6_tim_init_us(vet6_tim_t tim, uint32_t us)
{
    uint32_t clk = vet6_tim_get_clk(tim);
    uint32_t prescaler = (clk / 1000000) - 1; /* 1MHz计数频率 */
    uint32_t period = us - 1;

    vet6_tim_base_config_t config = {
        .tim = tim,
        .prescaler = prescaler,
        .period = period,
        .dir = VET6_TIM_DIR_UP,
        .auto_reload = true};

    return vet6_tim_base_init(&config);
}

/**
 * @brief 快速初始化定时器为毫秒级定时
 */
int vet6_tim_init_ms(vet6_tim_t tim, uint32_t ms)
{
    uint32_t clk = vet6_tim_get_clk(tim);
    uint32_t prescaler = (clk / 10000) - 1; /* 10kHz计数频率 */
    uint32_t period = (ms * 10) - 1;

    vet6_tim_base_config_t config = {
        .tim = tim,
        .prescaler = prescaler,
        .period = period,
        .dir = VET6_TIM_DIR_UP,
        .auto_reload = true};

    return vet6_tim_base_init(&config);
}

/**
 * @brief 启动定时器
 */
void vet6_tim_start(vet6_tim_t tim)
{
    if (tim >= VET6_TIM_MAX)
        return;
    tim_base_table[tim]->CR1 |= TIM_CR1_CEN;
}

/**
 * @brief 停止定时器
 */
void vet6_tim_stop(vet6_tim_t tim)
{
    if (tim >= VET6_TIM_MAX)
        return;
    tim_base_table[tim]->CR1 &= ~TIM_CR1_CEN;
}

/**
 * @brief 获取定时器计数值
 */
uint32_t vet6_tim_get_counter(vet6_tim_t tim)
{
    if (tim >= VET6_TIM_MAX)
        return 0;
    return tim_base_table[tim]->CNT;
}

/**
 * @brief 设置定时器计数值
 */
void vet6_tim_set_counter(vet6_tim_t tim, uint32_t value)
{
    if (tim >= VET6_TIM_MAX)
        return;
    tim_base_table[tim]->CNT = value;
}

/**
 * @brief 设置自动重载值
 */
void vet6_tim_set_period(vet6_tim_t tim, uint32_t value)
{
    if (tim >= VET6_TIM_MAX)
        return;
    tim_base_table[tim]->ARR = value;
}

/**
 * @brief 软件触发更新事件
 */
void vet6_tim_generate_update(vet6_tim_t tim)
{
    if (tim >= VET6_TIM_MAX)
        return;
    tim_base_table[tim]->EGR = TIM_EGR_UG;
}

/*===========================================================================*/
/*                              中断控制                                      */
/*===========================================================================*/

/**
 * @brief 使能定时器更新中断
 */
void vet6_tim_enable_update_irq(vet6_tim_t tim, uint8_t priority)
{
    if (tim >= VET6_TIM_MAX)
        return;

    TIM_TypeDef *t = tim_base_table[tim];

    /* 清除中断标志 */
    t->SR &= ~TIM_SR_UIF;

    /* 使能更新中断 */
    t->DIER |= TIM_DIER_UIE;

    /* 配置NVIC */
    NVIC_SetPriority(tim_irq_table[tim], priority);
    NVIC_EnableIRQ(tim_irq_table[tim]);
}

/**
 * @brief 禁用定时器更新中断
 */
void vet6_tim_disable_update_irq(vet6_tim_t tim)
{
    if (tim >= VET6_TIM_MAX)
        return;

    tim_base_table[tim]->DIER &= ~TIM_DIER_UIE;
    NVIC_DisableIRQ(tim_irq_table[tim]);
}

/**
 * @brief 清除更新中断标志
 */
void vet6_tim_clear_update_flag(vet6_tim_t tim)
{
    if (tim >= VET6_TIM_MAX)
        return;
    tim_base_table[tim]->SR &= ~TIM_SR_UIF;
}

/**
 * @brief 检查更新中断标志
 */
bool vet6_tim_check_update_flag(vet6_tim_t tim)
{
    if (tim >= VET6_TIM_MAX)
        return false;
    return (tim_base_table[tim]->SR & TIM_SR_UIF) ? true : false;
}

/*===========================================================================*/
/*                              PWM输出功能                                   */
/*===========================================================================*/

/**
 * @brief 获取CCR寄存器指针
 */
static __IO uint32_t *get_ccr_ptr(TIM_TypeDef *tim, vet6_tim_ch_t channel)
{
    switch (channel)
    {
    case VET6_TIM_CH1:
        return &tim->CCR1;
    case VET6_TIM_CH2:
        return &tim->CCR2;
    case VET6_TIM_CH3:
        return &tim->CCR3;
    case VET6_TIM_CH4:
        return &tim->CCR4;
    default:
        return NULL;
    }
}

/**
 * @brief 初始化PWM输出
 */
int vet6_pwm_init(const vet6_pwm_config_t *config)
{
    if (config == NULL || config->tim >= VET6_TIM_MAX)
        return -1;
    if (config->channel >= VET6_TIM_CH_MAX)
        return -1;

    TIM_TypeDef *tim = tim_base_table[config->tim];
    uint32_t clk = vet6_tim_get_clk(config->tim);

    /* 使能时钟 */
    vet6_tim_clk_enable(config->tim);

    /* 计算预分频和自动重载值 */
    uint32_t period = clk / config->frequency;
    uint32_t prescaler = 0;

    while (period > 65535 && prescaler < 65535)
    {
        prescaler++;
        period = clk / ((prescaler + 1) * config->frequency);
    }

    /* 基本配置 */
    tim->PSC = prescaler;
    tim->ARR = period - 1;
    tim->CR1 |= TIM_CR1_ARPE;

    /* 配置通道为PWM模式 */
    uint32_t ccmr_val = (config->mode == VET6_PWM_MODE1) ? (6 << 4) : (7 << 4); /* OCxM = 110 or 111 */
    ccmr_val |= (1 << 3);                                                       /* OCxPE = 1, 预装载使能 */

    uint32_t ccer_mask, ccer_val;

    switch (config->channel)
    {
    case VET6_TIM_CH1:
        tim->CCMR1 &= ~0x00FF;
        tim->CCMR1 |= ccmr_val;
        ccer_mask = TIM_CCER_CC1E | TIM_CCER_CC1P;
        ccer_val = TIM_CCER_CC1E | (config->polarity ? 0 : TIM_CCER_CC1P);
        break;
    case VET6_TIM_CH2:
        tim->CCMR1 &= ~0xFF00;
        tim->CCMR1 |= (ccmr_val << 8);
        ccer_mask = TIM_CCER_CC2E | TIM_CCER_CC2P;
        ccer_val = TIM_CCER_CC2E | (config->polarity ? 0 : TIM_CCER_CC2P);
        break;
    case VET6_TIM_CH3:
        tim->CCMR2 &= ~0x00FF;
        tim->CCMR2 |= ccmr_val;
        ccer_mask = TIM_CCER_CC3E | TIM_CCER_CC3P;
        ccer_val = TIM_CCER_CC3E | (config->polarity ? 0 : TIM_CCER_CC3P);
        break;
    case VET6_TIM_CH4:
        tim->CCMR2 &= ~0xFF00;
        tim->CCMR2 |= (ccmr_val << 8);
        ccer_mask = TIM_CCER_CC4E | TIM_CCER_CC4P;
        ccer_val = TIM_CCER_CC4E | (config->polarity ? 0 : TIM_CCER_CC4P);
        break;
    default:
        return -1;
    }

    tim->CCER &= ~ccer_mask;
    tim->CCER |= ccer_val;

    /* 设置占空比 */
    uint32_t ccr = (uint32_t)((uint64_t)(period)*config->duty / 1000);
    *get_ccr_ptr(tim, config->channel) = ccr;

    /* 高级定时器需要使能MOE */
    if (vet6_tim_is_advanced(config->tim))
    {
        tim->BDTR |= TIM_BDTR_MOE;
    }

    /* 触发更新事件 */
    tim->EGR = TIM_EGR_UG;

    return 0;
}

/**
 * @brief 快速初始化PWM
 */
int vet6_pwm_init_quick(vet6_tim_t tim, vet6_tim_ch_t channel,
                        uint32_t frequency, uint16_t duty)
{
    vet6_pwm_config_t config = {
        .tim = tim,
        .channel = channel,
        .frequency = frequency,
        .duty = duty,
        .mode = VET6_PWM_MODE1,
        .polarity = true};
    return vet6_pwm_init(&config);
}

/**
 * @brief 启动PWM输出
 */
void vet6_pwm_start(vet6_tim_t tim, vet6_tim_ch_t channel)
{
    if (tim >= VET6_TIM_MAX || channel >= VET6_TIM_CH_MAX)
        return;

    TIM_TypeDef *t = tim_base_table[tim];

    /* 使能通道输出 */
    t->CCER |= (TIM_CCER_CC1E << (channel * 4));

    /* 启动定时器 */
    t->CR1 |= TIM_CR1_CEN;
}

/**
 * @brief 停止PWM输出
 */
void vet6_pwm_stop(vet6_tim_t tim, vet6_tim_ch_t channel)
{
    if (tim >= VET6_TIM_MAX || channel >= VET6_TIM_CH_MAX)
        return;

    TIM_TypeDef *t = tim_base_table[tim];

    /* 禁用通道输出 */
    t->CCER &= ~(TIM_CCER_CC1E << (channel * 4));
}

/**
 * @brief 设置PWM占空比
 */
void vet6_pwm_set_duty(vet6_tim_t tim, vet6_tim_ch_t channel, uint16_t duty)
{
    if (tim >= VET6_TIM_MAX || channel >= VET6_TIM_CH_MAX)
        return;
    if (duty > VET6_PWM_DUTY_MAX)
        duty = VET6_PWM_DUTY_MAX;

    TIM_TypeDef *t = tim_base_table[tim];
    uint32_t arr = t->ARR + 1;
    uint32_t ccr = (uint32_t)((uint64_t)arr * duty / 1000);

    *get_ccr_ptr(t, channel) = ccr;
}

/**
 * @brief 设置PWM频率
 */
void vet6_pwm_set_frequency(vet6_tim_t tim, uint32_t frequency)
{
    if (tim >= VET6_TIM_MAX || frequency == 0)
        return;

    TIM_TypeDef *t = tim_base_table[tim];
    uint32_t clk = vet6_tim_get_clk(tim);
    uint32_t psc = t->PSC + 1;
    uint32_t arr = clk / (psc * frequency) - 1;

    t->ARR = arr;
}

/**
 * @brief 设置PWM比较值
 */
void vet6_pwm_set_compare(vet6_tim_t tim, vet6_tim_ch_t channel, uint32_t value)
{
    if (tim >= VET6_TIM_MAX || channel >= VET6_TIM_CH_MAX)
        return;
    *get_ccr_ptr(tim_base_table[tim], channel) = value;
}

/**
 * @brief 获取PWM比较值
 */
uint32_t vet6_pwm_get_compare(vet6_tim_t tim, vet6_tim_ch_t channel)
{
    if (tim >= VET6_TIM_MAX || channel >= VET6_TIM_CH_MAX)
        return 0;
    return *get_ccr_ptr(tim_base_table[tim], channel);
}

/*===========================================================================*/
/*                              编码器接口                                    */
/*===========================================================================*/

/**
 * @brief 初始化编码器接口
 */
int vet6_encoder_init(const vet6_encoder_config_t *config)
{
    if (config == NULL || config->tim >= VET6_TIM_MAX)
        return -1;

    /* 编码器模式仅支持TIM1-5, TIM8 */
    if (config->tim == VET6_TIM6 || config->tim == VET6_TIM7 ||
        config->tim >= VET6_TIM9)
    {
        return -1;
    }

    TIM_TypeDef *tim = tim_base_table[config->tim];

    /* 使能时钟 */
    vet6_tim_clk_enable(config->tim);

    /* 停止定时器 */
    tim->CR1 &= ~TIM_CR1_CEN;

    /* 配置编码器模式3 (TI1和TI2都计数) */
    tim->SMCR &= ~TIM_SMCR_SMS;
    tim->SMCR |= TIM_SMCR_SMS_0 | TIM_SMCR_SMS_1; /* SMS = 011 */

    /* 配置输入捕获通道1和2 */
    uint32_t filter = (config->filter & 0x0F) << 4;

    tim->CCMR1 = 0;
    tim->CCMR1 |= TIM_CCMR1_CC1S_0; /* CC1S = 01, TI1映射到IC1 */
    tim->CCMR1 |= TIM_CCMR1_CC2S_0; /* CC2S = 01, TI2映射到IC2 */
    tim->CCMR1 |= (filter << 0);    /* IC1滤波 */
    tim->CCMR1 |= (filter << 8);    /* IC2滤波 */

    /* 配置极性 */
    tim->CCER = 0;
    if (config->reverse)
    {
        tim->CCER |= TIM_CCER_CC1P; /* 反向 */
    }

    /* 设置计数范围 */
    if (vet6_tim_is_32bit(config->tim))
    {
        tim->ARR = 0xFFFFFFFF;
    }
    else
    {
        tim->ARR = 0xFFFF;
    }

    /* 复位计数器 */
    tim->CNT = 0;

    /* 启动定时器 */
    tim->CR1 |= TIM_CR1_CEN;

    return 0;
}

/**
 * @brief 获取编码器计数值
 */
int32_t vet6_encoder_get_count(vet6_tim_t tim)
{
    if (tim >= VET6_TIM_MAX)
        return 0;
    return (int32_t)tim_base_table[tim]->CNT;
}

/**
 * @brief 重置编码器计数
 */
void vet6_encoder_reset(vet6_tim_t tim)
{
    if (tim >= VET6_TIM_MAX)
        return;
    tim_base_table[tim]->CNT = 0;
}

/**
 * @brief 获取编码器方向
 */
uint8_t vet6_encoder_get_direction(vet6_tim_t tim)
{
    if (tim >= VET6_TIM_MAX)
        return 0;
    return (tim_base_table[tim]->CR1 & TIM_CR1_DIR) ? 1 : 0;
}

/*===========================================================================*/
/*                              高级定时器功能                                */
/*===========================================================================*/

/**
 * @brief 使能高级定时器主输出
 */
void vet6_tim_enable_moe(vet6_tim_t tim)
{
    if (!vet6_tim_is_advanced(tim))
        return;
    tim_base_table[tim]->BDTR |= TIM_BDTR_MOE;
}

/**
 * @brief 禁用高级定时器主输出
 */
void vet6_tim_disable_moe(vet6_tim_t tim)
{
    if (!vet6_tim_is_advanced(tim))
        return;
    tim_base_table[tim]->BDTR &= ~TIM_BDTR_MOE;
}

/**
 * @brief 配置死区时间
 */
void vet6_tim_set_deadtime(vet6_tim_t tim, uint8_t deadtime)
{
    if (!vet6_tim_is_advanced(tim))
        return;

    TIM_TypeDef *t = tim_base_table[tim];
    t->BDTR &= ~TIM_BDTR_DTG;
    t->BDTR |= deadtime;
}

/**
 * @brief 配置刹车功能
 */
void vet6_tim_config_break(vet6_tim_t tim, bool enable, bool polarity)
{
    if (!vet6_tim_is_advanced(tim))
        return;

    TIM_TypeDef *t = tim_base_table[tim];

    if (enable)
    {
        t->BDTR |= TIM_BDTR_BKE;
        if (polarity)
        {
            t->BDTR |= TIM_BDTR_BKP;
        }
        else
        {
            t->BDTR &= ~TIM_BDTR_BKP;
        }
    }
    else
    {
        t->BDTR &= ~TIM_BDTR_BKE;
    }
}
