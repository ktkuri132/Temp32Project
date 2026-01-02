/**
 * @file f407_adc.c
 * @brief STM32F407系列通用ADC板级驱动实现
 * @details 适用于STM32F407所有封装型号 (VET6/ZET6/VGT6/ZGT6/IGT6等)
 *          提供ADC1-ADC3的初始化和操作API
 */

#include "f407_adc.h"

/*===========================================================================*/
/*                              内部变量                                      */
/*===========================================================================*/

/* ADC外设基地址表 */
static ADC_TypeDef *const adc_base_table[F407_ADC_MAX] = {
    ADC1, ADC2, ADC3};

/* ADC中断号表 */
static const IRQn_Type adc_irq_table[F407_ADC_MAX] = {
    ADC_IRQn, ADC_IRQn, ADC_IRQn /* ADC1-3共享同一个中断 */
};

/* ADC通道对应的GPIO引脚表 */
static const f407_gpio_id_t adc_pin_table[] = {
    F407_PA0, // CH0
    F407_PA1, // CH1
    F407_PA2, // CH2
    F407_PA3, // CH3
    F407_PA4, // CH4
    F407_PA5, // CH5
    F407_PA6, // CH6
    F407_PA7, // CH7
    F407_PB0, // CH8
    F407_PB1, // CH9
    F407_PC0, // CH10
    F407_PC1, // CH11
    F407_PC2, // CH12
    F407_PC3, // CH13
    F407_PC4, // CH14
    F407_PC5  // CH15
};

/*===========================================================================*/
/*                              时钟控制                                      */
/*===========================================================================*/

/**
 * @brief 使能ADC时钟
 */
void f407_adc_clk_enable(f407_adc_t adc)
{
    switch (adc)
    {
    case F407_ADC1:
        RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
        break;
    case F407_ADC2:
        RCC->APB2ENR |= RCC_APB2ENR_ADC2EN;
        break;
    case F407_ADC3:
        RCC->APB2ENR |= RCC_APB2ENR_ADC3EN;
        break;
    default:
        break;
    }
    __DSB();
}

/**
 * @brief 禁用ADC时钟
 */
void f407_adc_clk_disable(f407_adc_t adc)
{
    switch (adc)
    {
    case F407_ADC1:
        RCC->APB2ENR &= ~RCC_APB2ENR_ADC1EN;
        break;
    case F407_ADC2:
        RCC->APB2ENR &= ~RCC_APB2ENR_ADC2EN;
        break;
    case F407_ADC3:
        RCC->APB2ENR &= ~RCC_APB2ENR_ADC3EN;
        break;
    default:
        break;
    }
}

/**
 * @brief 获取ADC外设基地址
 */
ADC_TypeDef *f407_adc_get_base(f407_adc_t adc)
{
    if (adc >= F407_ADC_MAX)
        return NULL;
    return adc_base_table[adc];
}

/*===========================================================================*/
/*                              引脚配置                                      */
/*===========================================================================*/

/**
 * @brief 配置ADC通道引脚为模拟模式
 */
int f407_adc_pin_init(f407_adc_channel_t channel)
{
    /* 内部通道不需要配置GPIO */
    if (channel >= F407_ADC_CH_TEMP)
    {
        return 0;
    }

    if (channel > F407_ADC_CH15)
    {
        return -1;
    }

    /* 配置引脚为模拟模式 */
    return f407_gpio_config_analog(adc_pin_table[channel]);
}

/*===========================================================================*/
/*                              初始化函数                                    */
/*===========================================================================*/

/**
 * @brief 初始化ADC
 */
int f407_adc_init(f407_adc_handle_t *handle, const f407_adc_config_t *config)
{
    if (handle == NULL || config == NULL)
        return -1;
    if (config->adc >= F407_ADC_MAX)
        return -1;

    /* 保存配置 */
    handle->config = *config;
    handle->instance = adc_base_table[config->adc];
    handle->initialized = false;

    ADC_TypeDef *adc = handle->instance;

    /* 使能时钟 */
    f407_adc_clk_enable(config->adc);

    /* 配置公共控制寄存器 (ADC预分频) */
    ADC->CCR &= ~ADC_CCR_ADCPRE;
    ADC->CCR |= (F407_ADC_PRESCALER_DIV4 << 16); /* APB2/4 = 84/4 = 21MHz */

    /* 禁用ADC进行配置 */
    adc->CR2 &= ~ADC_CR2_ADON;

    /* 配置分辨率 */
    adc->CR1 &= ~ADC_CR1_RES;
    adc->CR1 |= ((uint32_t)config->resolution << 24);

    /* 配置对齐方式 */
    if (config->align_left)
    {
        adc->CR2 |= ADC_CR2_ALIGN;
    }
    else
    {
        adc->CR2 &= ~ADC_CR2_ALIGN;
    }

    /* 配置转换模式 */
    switch (config->mode)
    {
    case F407_ADC_MODE_SINGLE:
        adc->CR2 &= ~ADC_CR2_CONT;
        adc->CR1 &= ~ADC_CR1_SCAN;
        break;
    case F407_ADC_MODE_CONTINUOUS:
        adc->CR2 |= ADC_CR2_CONT;
        adc->CR1 &= ~ADC_CR1_SCAN;
        break;
    case F407_ADC_MODE_SCAN:
        adc->CR2 &= ~ADC_CR2_CONT;
        adc->CR1 |= ADC_CR1_SCAN;
        break;
    }

    /* DMA配置 */
    if (config->use_dma)
    {
        adc->CR2 |= ADC_CR2_DMA | ADC_CR2_DDS;
    }
    else
    {
        adc->CR2 &= ~(ADC_CR2_DMA | ADC_CR2_DDS);
    }

    /* 默认使用软件触发 */
    adc->CR2 &= ~ADC_CR2_EXTEN;

    /* 设置规则通道序列长度为1 */
    adc->SQR1 &= ~ADC_SQR1_L;

    /* 使能ADC */
    adc->CR2 |= ADC_CR2_ADON;

    handle->initialized = true;

    return 0;
}

/**
 * @brief 快速初始化ADC
 */
int f407_adc_init_simple(f407_adc_handle_t *handle, f407_adc_t adc)
{
    f407_adc_config_t config = {
        .adc = adc,
        .resolution = F407_ADC_RES_12BIT,
        .mode = F407_ADC_MODE_SINGLE,
        .align_left = false,
        .use_dma = false};

    return f407_adc_init(handle, &config);
}

/**
 * @brief 反初始化ADC
 */
void f407_adc_deinit(f407_adc_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;

    /* 禁用ADC */
    handle->instance->CR2 &= ~ADC_CR2_ADON;

    /* 禁用时钟 */
    f407_adc_clk_disable(handle->config.adc);

    handle->initialized = false;
}

/*===========================================================================*/
/*                              基本操作                                      */
/*===========================================================================*/

/**
 * @brief 使能ADC
 */
void f407_adc_enable(f407_adc_handle_t *handle)
{
    if (handle == NULL)
        return;
    handle->instance->CR2 |= ADC_CR2_ADON;
}

/**
 * @brief 禁用ADC
 */
void f407_adc_disable(f407_adc_handle_t *handle)
{
    if (handle == NULL)
        return;
    handle->instance->CR2 &= ~ADC_CR2_ADON;
}

/**
 * @brief 配置ADC通道
 */
int f407_adc_config_channel(f407_adc_handle_t *handle, f407_adc_channel_t channel,
                            uint8_t rank, f407_adc_sampletime_t sampletime)
{
    if (handle == NULL || !handle->initialized)
        return -1;
    if (rank < 1 || rank > 16)
        return -1;
    if (channel >= F407_ADC_CH_MAX)
        return -1;

    ADC_TypeDef *adc = handle->instance;

    /* 配置采样时间 */
    if (channel <= F407_ADC_CH9)
    {
        adc->SMPR2 &= ~(0x07 << (channel * 3));
        adc->SMPR2 |= ((uint32_t)sampletime << (channel * 3));
    }
    else
    {
        adc->SMPR1 &= ~(0x07 << ((channel - 10) * 3));
        adc->SMPR1 |= ((uint32_t)sampletime << ((channel - 10) * 3));
    }

    /* 配置规则序列 */
    if (rank <= 6)
    {
        adc->SQR3 &= ~(0x1F << ((rank - 1) * 5));
        adc->SQR3 |= ((uint32_t)channel << ((rank - 1) * 5));
    }
    else if (rank <= 12)
    {
        adc->SQR2 &= ~(0x1F << ((rank - 7) * 5));
        adc->SQR2 |= ((uint32_t)channel << ((rank - 7) * 5));
    }
    else
    {
        adc->SQR1 &= ~(0x1F << ((rank - 13) * 5));
        adc->SQR1 |= ((uint32_t)channel << ((rank - 13) * 5));
    }

    return 0;
}

/**
 * @brief 启动ADC转换
 */
void f407_adc_start(f407_adc_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;

    /* 清除EOC标志 */
    handle->instance->SR &= ~ADC_SR_EOC;

    /* 启动转换 */
    handle->instance->CR2 |= ADC_CR2_SWSTART;
}

/**
 * @brief 停止ADC转换
 */
void f407_adc_stop(f407_adc_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;

    /* 停止连续转换 */
    handle->instance->CR2 &= ~ADC_CR2_CONT;
}

/**
 * @brief 等待转换完成
 */
int f407_adc_wait_eoc(f407_adc_handle_t *handle, uint32_t timeout)
{
    if (handle == NULL || !handle->initialized)
        return -1;

    while (!(handle->instance->SR & ADC_SR_EOC))
    {
        if (--timeout == 0)
            return -1;
    }

    return 0;
}

/**
 * @brief 读取ADC转换结果
 */
uint16_t f407_adc_read(f407_adc_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return 0;
    return (uint16_t)handle->instance->DR;
}

/**
 * @brief 单次转换并读取结果
 */
uint16_t f407_adc_read_channel(f407_adc_handle_t *handle, f407_adc_channel_t channel)
{
    if (handle == NULL || !handle->initialized)
        return 0;

    /* 配置引脚 */
    f407_adc_pin_init(channel);

    /* 配置通道 */
    f407_adc_config_channel(handle, channel, 1, F407_ADC_SAMPLETIME_84);

    /* 启动转换 */
    f407_adc_start(handle);

    /* 等待完成 */
    if (f407_adc_wait_eoc(handle, 10000) != 0)
    {
        return 0;
    }

    /* 读取结果 */
    return f407_adc_read(handle);
}

/**
 * @brief 多次采样取平均值
 */
uint16_t f407_adc_read_average(f407_adc_handle_t *handle, f407_adc_channel_t channel,
                               uint8_t samples)
{
    if (handle == NULL || !handle->initialized || samples == 0)
        return 0;

    uint32_t sum = 0;

    for (uint8_t i = 0; i < samples; i++)
    {
        sum += f407_adc_read_channel(handle, channel);
    }

    return (uint16_t)(sum / samples);
}

/*===========================================================================*/
/*                              特殊功能                                      */
/*===========================================================================*/

/**
 * @brief 使能内部温度传感器和VREFINT
 */
void f407_adc_enable_temp_vref(void)
{
    ADC->CCR |= ADC_CCR_TSVREFE;
}

/**
 * @brief 禁用内部温度传感器和VREFINT
 */
void f407_adc_disable_temp_vref(void)
{
    ADC->CCR &= ~ADC_CCR_TSVREFE;
}

/**
 * @brief 使能VBAT测量
 */
void f407_adc_enable_vbat(void)
{
    ADC->CCR |= ADC_CCR_VBATE;
}

/**
 * @brief 禁用VBAT测量
 */
void f407_adc_disable_vbat(void)
{
    ADC->CCR &= ~ADC_CCR_VBATE;
}

/**
 * @brief 读取内部温度
 */
int16_t f407_adc_read_temperature(f407_adc_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return 0;
    if (handle->config.adc != F407_ADC1)
        return 0; /* 仅ADC1支持 */

    /* 使能温度传感器 */
    f407_adc_enable_temp_vref();

    /* 读取温度通道 (需要较长采样时间) */
    f407_adc_config_channel(handle, F407_ADC_CH_TEMP, 1, F407_ADC_SAMPLETIME_480);

    f407_adc_start(handle);
    if (f407_adc_wait_eoc(handle, 10000) != 0)
    {
        return 0;
    }

    uint16_t adc_value = f407_adc_read(handle);

    /* 计算温度: T = ((Vsense - V25) / Avg_Slope) + 25 */
    /* Vsense = adc_value * 3300 / 4095 */
    int32_t vsense = (int32_t)adc_value * 3300 / 4095;
    int16_t temp = (int16_t)(((vsense - F407_TEMP_V25) * 10 / F407_TEMP_AVG_SLOPE) + 250);

    return temp;
}

/**
 * @brief 读取内部参考电压
 */
uint16_t f407_adc_read_vrefint(f407_adc_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return 0;
    if (handle->config.adc != F407_ADC1)
        return 0;

    /* 使能VREFINT */
    f407_adc_enable_temp_vref();

    /* 读取VREFINT通道 */
    f407_adc_config_channel(handle, F407_ADC_CH_VREFINT, 1, F407_ADC_SAMPLETIME_480);

    f407_adc_start(handle);
    if (f407_adc_wait_eoc(handle, 10000) != 0)
    {
        return 0;
    }

    return f407_adc_read(handle);
}

/**
 * @brief 读取VBAT电压
 */
uint16_t f407_adc_read_vbat(f407_adc_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return 0;
    if (handle->config.adc != F407_ADC1)
        return 0;

    /* 使能VBAT */
    f407_adc_enable_vbat();

    /* 读取VBAT通道 */
    f407_adc_config_channel(handle, F407_ADC_CH_VBAT, 1, F407_ADC_SAMPLETIME_480);

    f407_adc_start(handle);
    if (f407_adc_wait_eoc(handle, 10000) != 0)
    {
        return 0;
    }

    uint16_t adc_value = f407_adc_read(handle);

    /* VBAT = ADC_value * VREF / 4095 * 2 (内部分压) */
    return (uint16_t)((uint32_t)adc_value * 3300 * 2 / 4095);
}

/**
 * @brief ADC值转换为电压
 */
uint16_t f407_adc_to_voltage(f407_adc_handle_t *handle, uint16_t adc_value, uint16_t vref_mv)
{
    if (handle == NULL)
        return 0;

    uint16_t max_value;
    switch (handle->config.resolution)
    {
    case F407_ADC_RES_12BIT:
        max_value = F407_ADC_MAX_VALUE_12BIT;
        break;
    case F407_ADC_RES_10BIT:
        max_value = F407_ADC_MAX_VALUE_10BIT;
        break;
    case F407_ADC_RES_8BIT:
        max_value = F407_ADC_MAX_VALUE_8BIT;
        break;
    case F407_ADC_RES_6BIT:
        max_value = F407_ADC_MAX_VALUE_6BIT;
        break;
    default:
        max_value = F407_ADC_MAX_VALUE_12BIT;
        break;
    }

    return (uint16_t)((uint32_t)adc_value * vref_mv / max_value);
}

/*===========================================================================*/
/*                              中断/DMA                                      */
/*===========================================================================*/

/**
 * @brief 使能ADC转换完成中断
 */
void f407_adc_enable_eoc_irq(f407_adc_handle_t *handle, uint8_t priority)
{
    if (handle == NULL || !handle->initialized)
        return;

    /* 清除标志 */
    handle->instance->SR &= ~ADC_SR_EOC;

    /* 使能EOC中断 */
    handle->instance->CR1 |= ADC_CR1_EOCIE;

    /* 配置NVIC */
    NVIC_SetPriority(adc_irq_table[handle->config.adc], priority);
    NVIC_EnableIRQ(adc_irq_table[handle->config.adc]);
}

/**
 * @brief 禁用ADC转换完成中断
 */
void f407_adc_disable_eoc_irq(f407_adc_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;

    handle->instance->CR1 &= ~ADC_CR1_EOCIE;
}

/**
 * @brief 使能ADC DMA
 */
void f407_adc_enable_dma(f407_adc_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;
    handle->instance->CR2 |= ADC_CR2_DMA | ADC_CR2_DDS;
}

/**
 * @brief 禁用ADC DMA
 */
void f407_adc_disable_dma(f407_adc_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;
    handle->instance->CR2 &= ~(ADC_CR2_DMA | ADC_CR2_DDS);
}
