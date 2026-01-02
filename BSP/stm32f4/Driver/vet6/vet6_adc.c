/**
 * @file vet6_adc.c
 * @brief STM32F407VET6 ADC板级驱动实现
 * @details 提供ADC1-ADC3的初始化和操作API
 */

#include "vet6_adc.h"

/*===========================================================================*/
/*                              内部变量                                      */
/*===========================================================================*/

/* ADC外设基地址表 */
static ADC_TypeDef *const adc_base_table[VET6_ADC_MAX] = {
    ADC1, ADC2, ADC3};

/* ADC中断号表 */
static const IRQn_Type adc_irq_table[VET6_ADC_MAX] = {
    ADC_IRQn, ADC_IRQn, ADC_IRQn /* ADC1-3共享同一个中断 */
};

/* ADC通道对应的GPIO引脚表 */
static const vet6_gpio_id_t adc_pin_table[] = {
    VET6_PA0, // CH0
    VET6_PA1, // CH1
    VET6_PA2, // CH2
    VET6_PA3, // CH3
    VET6_PA4, // CH4
    VET6_PA5, // CH5
    VET6_PA6, // CH6
    VET6_PA7, // CH7
    VET6_PB0, // CH8
    VET6_PB1, // CH9
    VET6_PC0, // CH10
    VET6_PC1, // CH11
    VET6_PC2, // CH12
    VET6_PC3, // CH13
    VET6_PC4, // CH14
    VET6_PC5  // CH15
};

/*===========================================================================*/
/*                              时钟控制                                      */
/*===========================================================================*/

/**
 * @brief 使能ADC时钟
 */
void vet6_adc_clk_enable(vet6_adc_t adc)
{
    switch (adc)
    {
    case VET6_ADC1:
        RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
        break;
    case VET6_ADC2:
        RCC->APB2ENR |= RCC_APB2ENR_ADC2EN;
        break;
    case VET6_ADC3:
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
void vet6_adc_clk_disable(vet6_adc_t adc)
{
    switch (adc)
    {
    case VET6_ADC1:
        RCC->APB2ENR &= ~RCC_APB2ENR_ADC1EN;
        break;
    case VET6_ADC2:
        RCC->APB2ENR &= ~RCC_APB2ENR_ADC2EN;
        break;
    case VET6_ADC3:
        RCC->APB2ENR &= ~RCC_APB2ENR_ADC3EN;
        break;
    default:
        break;
    }
}

/**
 * @brief 获取ADC外设基地址
 */
ADC_TypeDef *vet6_adc_get_base(vet6_adc_t adc)
{
    if (adc >= VET6_ADC_MAX)
        return NULL;
    return adc_base_table[adc];
}

/*===========================================================================*/
/*                              引脚配置                                      */
/*===========================================================================*/

/**
 * @brief 配置ADC通道引脚为模拟模式
 */
int vet6_adc_pin_init(vet6_adc_channel_t channel)
{
    /* 内部通道不需要配置GPIO */
    if (channel >= VET6_ADC_CH_TEMP)
    {
        return 0;
    }

    if (channel > VET6_ADC_CH15)
    {
        return -1;
    }

    /* 配置引脚为模拟模式 */
    return vet6_gpio_config_analog(adc_pin_table[channel]);
}

/*===========================================================================*/
/*                              初始化函数                                    */
/*===========================================================================*/

/**
 * @brief 初始化ADC
 */
int vet6_adc_init(vet6_adc_handle_t *handle, const vet6_adc_config_t *config)
{
    if (handle == NULL || config == NULL)
        return -1;
    if (config->adc >= VET6_ADC_MAX)
        return -1;

    /* 保存配置 */
    handle->config = *config;
    handle->instance = adc_base_table[config->adc];
    handle->initialized = false;

    ADC_TypeDef *adc = handle->instance;

    /* 使能时钟 */
    vet6_adc_clk_enable(config->adc);

    /* 配置公共控制寄存器 (ADC预分频) */
    ADC->CCR &= ~ADC_CCR_ADCPRE;
    ADC->CCR |= (VET6_ADC_PRESCALER_DIV4 << 16); /* APB2/4 = 84/4 = 21MHz */

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
    case VET6_ADC_MODE_SINGLE:
        adc->CR2 &= ~ADC_CR2_CONT;
        adc->CR1 &= ~ADC_CR1_SCAN;
        break;
    case VET6_ADC_MODE_CONTINUOUS:
        adc->CR2 |= ADC_CR2_CONT;
        adc->CR1 &= ~ADC_CR1_SCAN;
        break;
    case VET6_ADC_MODE_SCAN:
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
int vet6_adc_init_simple(vet6_adc_handle_t *handle, vet6_adc_t adc)
{
    vet6_adc_config_t config = {
        .adc = adc,
        .resolution = VET6_ADC_RES_12BIT,
        .mode = VET6_ADC_MODE_SINGLE,
        .align_left = false,
        .use_dma = false};

    return vet6_adc_init(handle, &config);
}

/**
 * @brief 反初始化ADC
 */
void vet6_adc_deinit(vet6_adc_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;

    /* 禁用ADC */
    handle->instance->CR2 &= ~ADC_CR2_ADON;

    /* 禁用时钟 */
    vet6_adc_clk_disable(handle->config.adc);

    handle->initialized = false;
}

/*===========================================================================*/
/*                              基本操作                                      */
/*===========================================================================*/

/**
 * @brief 使能ADC
 */
void vet6_adc_enable(vet6_adc_handle_t *handle)
{
    if (handle == NULL)
        return;
    handle->instance->CR2 |= ADC_CR2_ADON;
}

/**
 * @brief 禁用ADC
 */
void vet6_adc_disable(vet6_adc_handle_t *handle)
{
    if (handle == NULL)
        return;
    handle->instance->CR2 &= ~ADC_CR2_ADON;
}

/**
 * @brief 配置ADC通道
 */
int vet6_adc_config_channel(vet6_adc_handle_t *handle, vet6_adc_channel_t channel,
                            uint8_t rank, vet6_adc_sampletime_t sampletime)
{
    if (handle == NULL || !handle->initialized)
        return -1;
    if (rank < 1 || rank > 16)
        return -1;
    if (channel >= VET6_ADC_CH_MAX)
        return -1;

    ADC_TypeDef *adc = handle->instance;

    /* 配置采样时间 */
    if (channel <= VET6_ADC_CH9)
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
void vet6_adc_start(vet6_adc_handle_t *handle)
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
void vet6_adc_stop(vet6_adc_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;

    /* 停止连续转换 */
    handle->instance->CR2 &= ~ADC_CR2_CONT;
}

/**
 * @brief 等待转换完成
 */
int vet6_adc_wait_eoc(vet6_adc_handle_t *handle, uint32_t timeout)
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
uint16_t vet6_adc_read(vet6_adc_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return 0;
    return (uint16_t)handle->instance->DR;
}

/**
 * @brief 单次转换并读取结果
 */
uint16_t vet6_adc_read_channel(vet6_adc_handle_t *handle, vet6_adc_channel_t channel)
{
    if (handle == NULL || !handle->initialized)
        return 0;

    /* 配置引脚 */
    vet6_adc_pin_init(channel);

    /* 配置通道 */
    vet6_adc_config_channel(handle, channel, 1, VET6_ADC_SAMPLETIME_84);

    /* 启动转换 */
    vet6_adc_start(handle);

    /* 等待完成 */
    if (vet6_adc_wait_eoc(handle, 10000) != 0)
    {
        return 0;
    }

    /* 读取结果 */
    return vet6_adc_read(handle);
}

/**
 * @brief 多次采样取平均值
 */
uint16_t vet6_adc_read_average(vet6_adc_handle_t *handle, vet6_adc_channel_t channel,
                               uint8_t samples)
{
    if (handle == NULL || !handle->initialized || samples == 0)
        return 0;

    uint32_t sum = 0;

    for (uint8_t i = 0; i < samples; i++)
    {
        sum += vet6_adc_read_channel(handle, channel);
    }

    return (uint16_t)(sum / samples);
}

/*===========================================================================*/
/*                              特殊功能                                      */
/*===========================================================================*/

/**
 * @brief 使能内部温度传感器和VREFINT
 */
void vet6_adc_enable_temp_vref(void)
{
    ADC->CCR |= ADC_CCR_TSVREFE;
}

/**
 * @brief 禁用内部温度传感器和VREFINT
 */
void vet6_adc_disable_temp_vref(void)
{
    ADC->CCR &= ~ADC_CCR_TSVREFE;
}

/**
 * @brief 使能VBAT测量
 */
void vet6_adc_enable_vbat(void)
{
    ADC->CCR |= ADC_CCR_VBATE;
}

/**
 * @brief 禁用VBAT测量
 */
void vet6_adc_disable_vbat(void)
{
    ADC->CCR &= ~ADC_CCR_VBATE;
}

/**
 * @brief 读取内部温度
 */
int16_t vet6_adc_read_temperature(vet6_adc_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return 0;
    if (handle->config.adc != VET6_ADC1)
        return 0; /* 仅ADC1支持 */

    /* 使能温度传感器 */
    vet6_adc_enable_temp_vref();

    /* 读取温度通道 (需要较长采样时间) */
    vet6_adc_config_channel(handle, VET6_ADC_CH_TEMP, 1, VET6_ADC_SAMPLETIME_480);

    vet6_adc_start(handle);
    if (vet6_adc_wait_eoc(handle, 10000) != 0)
    {
        return 0;
    }

    uint16_t adc_value = vet6_adc_read(handle);

    /* 计算温度: T = ((Vsense - V25) / Avg_Slope) + 25 */
    /* Vsense = adc_value * 3300 / 4095 */
    int32_t vsense = (int32_t)adc_value * 3300 / 4095;
    int16_t temp = (int16_t)(((vsense - VET6_TEMP_V25) * 10 / VET6_TEMP_AVG_SLOPE) + 250);

    return temp;
}

/**
 * @brief 读取内部参考电压
 */
uint16_t vet6_adc_read_vrefint(vet6_adc_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return 0;
    if (handle->config.adc != VET6_ADC1)
        return 0;

    /* 使能VREFINT */
    vet6_adc_enable_temp_vref();

    /* 读取VREFINT通道 */
    vet6_adc_config_channel(handle, VET6_ADC_CH_VREFINT, 1, VET6_ADC_SAMPLETIME_480);

    vet6_adc_start(handle);
    if (vet6_adc_wait_eoc(handle, 10000) != 0)
    {
        return 0;
    }

    return vet6_adc_read(handle);
}

/**
 * @brief 读取VBAT电压
 */
uint16_t vet6_adc_read_vbat(vet6_adc_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return 0;
    if (handle->config.adc != VET6_ADC1)
        return 0;

    /* 使能VBAT */
    vet6_adc_enable_vbat();

    /* 读取VBAT通道 */
    vet6_adc_config_channel(handle, VET6_ADC_CH_VBAT, 1, VET6_ADC_SAMPLETIME_480);

    vet6_adc_start(handle);
    if (vet6_adc_wait_eoc(handle, 10000) != 0)
    {
        return 0;
    }

    uint16_t adc_value = vet6_adc_read(handle);

    /* VBAT = ADC_value * VREF / 4095 * 2 (内部分压) */
    return (uint16_t)((uint32_t)adc_value * 3300 * 2 / 4095);
}

/**
 * @brief ADC值转换为电压
 */
uint16_t vet6_adc_to_voltage(vet6_adc_handle_t *handle, uint16_t adc_value, uint16_t vref_mv)
{
    if (handle == NULL)
        return 0;

    uint16_t max_value;
    switch (handle->config.resolution)
    {
    case VET6_ADC_RES_12BIT:
        max_value = VET6_ADC_MAX_VALUE_12BIT;
        break;
    case VET6_ADC_RES_10BIT:
        max_value = VET6_ADC_MAX_VALUE_10BIT;
        break;
    case VET6_ADC_RES_8BIT:
        max_value = VET6_ADC_MAX_VALUE_8BIT;
        break;
    case VET6_ADC_RES_6BIT:
        max_value = VET6_ADC_MAX_VALUE_6BIT;
        break;
    default:
        max_value = VET6_ADC_MAX_VALUE_12BIT;
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
void vet6_adc_enable_eoc_irq(vet6_adc_handle_t *handle, uint8_t priority)
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
void vet6_adc_disable_eoc_irq(vet6_adc_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;

    handle->instance->CR1 &= ~ADC_CR1_EOCIE;
}

/**
 * @brief 使能ADC DMA
 */
void vet6_adc_enable_dma(vet6_adc_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;
    handle->instance->CR2 |= ADC_CR2_DMA | ADC_CR2_DDS;
}

/**
 * @brief 禁用ADC DMA
 */
void vet6_adc_disable_dma(vet6_adc_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;
    handle->instance->CR2 &= ~(ADC_CR2_DMA | ADC_CR2_DDS);
}
