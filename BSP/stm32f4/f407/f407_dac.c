/**
 * @file f407_dac.c
 * @brief STM32F407系列 DAC板级驱动实现
 * @details 提供DAC1/DAC2通道的初始化和操作API
 *          适用于STM32F407所有封装型号 (VET6, ZGT6, ZET6, VGT6等)
 */

#include "f407_dac.h"

/*===========================================================================*/
/*                              内部常量                                      */
/*===========================================================================*/

/* DAC通道对应的GPIO引脚 */
static const f407_gpio_id_t dac_pin_table[F407_DAC_CH_MAX] = {
    F407_PA4, // DAC_CH1
    F407_PA5  // DAC_CH2
};

/*===========================================================================*/
/*                              时钟控制                                      */
/*===========================================================================*/

/**
 * @brief 使能DAC时钟
 */
void f407_dac_clk_enable(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_DACEN;
    __DSB();
}

/**
 * @brief 禁用DAC时钟
 */
void f407_dac_clk_disable(void)
{
    RCC->APB1ENR &= ~RCC_APB1ENR_DACEN;
}

/*===========================================================================*/
/*                              初始化函数                                    */
/*===========================================================================*/

/**
 * @brief 初始化DAC通道
 */
int f407_dac_init(f407_dac_handle_t *handle, const f407_dac_config_t *config)
{
    if (handle == NULL || config == NULL)
        return -1;
    if (config->channel >= F407_DAC_CH_MAX)
        return -1;

    /* 保存配置 */
    handle->config = *config;
    handle->initialized = false;

    /* 使能DAC时钟 */
    f407_dac_clk_enable();

    /* 配置引脚为模拟模式 */
    f407_gpio_config_analog(dac_pin_table[config->channel]);

    /* 获取控制寄存器偏移 */
    uint32_t cr_offset = config->channel * 16;

    /* 清除通道配置 */
    DAC->CR &= ~(0xFFFF << cr_offset);

    uint32_t cr_value = 0;

    /* 配置输出缓冲 */
    if (!config->output_buffer)
    {
        cr_value |= DAC_CR_BOFF1; /* 禁用缓冲 */
    }

    /* 配置触发源 */
    if (config->trigger != F407_DAC_TRIG_NONE)
    {
        cr_value |= DAC_CR_TEN1; /* 使能触发 */

        if (config->trigger == F407_DAC_TRIG_SOFTWARE)
        {
            cr_value |= (7 << 3); /* TSEL = 111 软件触发 */
        }
        else
        {
            cr_value |= ((config->trigger - 1) << 3);
        }
    }

    /* 配置波形生成 */
    switch (config->wave_gen)
    {
    case F407_DAC_WAVE_NOISE:
        cr_value |= DAC_CR_WAVE1_0;
        cr_value |= ((uint32_t)config->triangle_amp << 8); /* MAMP */
        break;
    case F407_DAC_WAVE_TRIANGLE:
        cr_value |= DAC_CR_WAVE1_1;
        cr_value |= ((uint32_t)config->triangle_amp << 8); /* MAMP */
        break;
    default:
        break;
    }

    /* DMA配置 */
    if (config->use_dma)
    {
        cr_value |= DAC_CR_DMAEN1;
    }

    /* 写入配置 */
    DAC->CR |= (cr_value << cr_offset);

    handle->initialized = true;

    return 0;
}

/**
 * @brief 快速初始化DAC通道
 */
int f407_dac_init_simple(f407_dac_handle_t *handle, f407_dac_channel_t channel)
{
    f407_dac_config_t config = {
        .channel = channel,
        .trigger = F407_DAC_TRIG_NONE,
        .wave_gen = F407_DAC_WAVE_NONE,
        .triangle_amp = F407_DAC_TRIANGLE_AMP_1,
        .output_buffer = true,
        .use_dma = false};

    return f407_dac_init(handle, &config);
}

/**
 * @brief 反初始化DAC通道
 */
void f407_dac_deinit(f407_dac_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;

    /* 禁用通道 */
    f407_dac_disable(handle);

    /* 清除通道配置 */
    uint32_t cr_offset = handle->config.channel * 16;
    DAC->CR &= ~(0xFFFF << cr_offset);

    handle->initialized = false;
}

/*===========================================================================*/
/*                              基本操作                                      */
/*===========================================================================*/

/**
 * @brief 使能DAC通道
 */
void f407_dac_enable(f407_dac_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;

    if (handle->config.channel == F407_DAC_CH1)
    {
        DAC->CR |= DAC_CR_EN1;
    }
    else
    {
        DAC->CR |= DAC_CR_EN2;
    }
}

/**
 * @brief 禁用DAC通道
 */
void f407_dac_disable(f407_dac_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;

    if (handle->config.channel == F407_DAC_CH1)
    {
        DAC->CR &= ~DAC_CR_EN1;
    }
    else
    {
        DAC->CR &= ~DAC_CR_EN2;
    }
}

/**
 * @brief 设置DAC输出值 (12位右对齐)
 */
void f407_dac_set_value(f407_dac_handle_t *handle, uint16_t value)
{
    if (handle == NULL || !handle->initialized)
        return;

    if (value > F407_DAC_MAX_VALUE_12BIT)
    {
        value = F407_DAC_MAX_VALUE_12BIT;
    }

    if (handle->config.channel == F407_DAC_CH1)
    {
        DAC->DHR12R1 = value;
    }
    else
    {
        DAC->DHR12R2 = value;
    }
}

/**
 * @brief 设置DAC输出值 (指定对齐方式)
 */
void f407_dac_set_value_aligned(f407_dac_handle_t *handle, uint16_t value,
                                f407_dac_align_t align)
{
    if (handle == NULL || !handle->initialized)
        return;

    if (handle->config.channel == F407_DAC_CH1)
    {
        switch (align)
        {
        case F407_DAC_ALIGN_12B_R:
            DAC->DHR12R1 = value & 0x0FFF;
            break;
        case F407_DAC_ALIGN_12B_L:
            DAC->DHR12L1 = value & 0xFFF0;
            break;
        case F407_DAC_ALIGN_8B_R:
            DAC->DHR8R1 = value & 0xFF;
            break;
        }
    }
    else
    {
        switch (align)
        {
        case F407_DAC_ALIGN_12B_R:
            DAC->DHR12R2 = value & 0x0FFF;
            break;
        case F407_DAC_ALIGN_12B_L:
            DAC->DHR12L2 = value & 0xFFF0;
            break;
        case F407_DAC_ALIGN_8B_R:
            DAC->DHR8R2 = value & 0xFF;
            break;
        }
    }
}

/**
 * @brief 获取DAC输出值
 */
uint16_t f407_dac_get_value(f407_dac_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return 0;

    if (handle->config.channel == F407_DAC_CH1)
    {
        return (uint16_t)(DAC->DOR1 & 0x0FFF);
    }
    else
    {
        return (uint16_t)(DAC->DOR2 & 0x0FFF);
    }
}

/**
 * @brief 设置DAC输出电压 (mV)
 */
void f407_dac_set_voltage(f407_dac_handle_t *handle, uint16_t voltage_mv)
{
    if (handle == NULL || !handle->initialized)
        return;

    if (voltage_mv > F407_DAC_VREF_MV)
    {
        voltage_mv = F407_DAC_VREF_MV;
    }

    uint16_t value = (uint16_t)((uint32_t)voltage_mv * F407_DAC_MAX_VALUE_12BIT / F407_DAC_VREF_MV);
    f407_dac_set_value(handle, value);
}

/**
 * @brief 软件触发DAC转换
 */
void f407_dac_software_trigger(f407_dac_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;

    if (handle->config.channel == F407_DAC_CH1)
    {
        DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1;
    }
    else
    {
        DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG2;
    }
}

/*===========================================================================*/
/*                              波形生成                                      */
/*===========================================================================*/

/**
 * @brief 使能噪声波形生成
 */
void f407_dac_enable_noise(f407_dac_handle_t *handle, uint8_t lfsr_mask)
{
    if (handle == NULL || !handle->initialized)
        return;
    if (lfsr_mask > 11)
        lfsr_mask = 11;

    uint32_t cr_offset = handle->config.channel * 16;

    /* 清除波形和幅度位 */
    DAC->CR &= ~((DAC_CR_WAVE1 | DAC_CR_MAMP1) << cr_offset);

    /* 设置噪声波形和LFSR掩码 */
    DAC->CR |= ((DAC_CR_WAVE1_0 | ((uint32_t)lfsr_mask << 8)) << cr_offset);
}

/**
 * @brief 使能三角波生成
 */
void f407_dac_enable_triangle(f407_dac_handle_t *handle, f407_dac_triangle_amp_t amplitude)
{
    if (handle == NULL || !handle->initialized)
        return;

    uint32_t cr_offset = handle->config.channel * 16;

    /* 清除波形和幅度位 */
    DAC->CR &= ~((DAC_CR_WAVE1 | DAC_CR_MAMP1) << cr_offset);

    /* 设置三角波和幅度 */
    DAC->CR |= ((DAC_CR_WAVE1_1 | ((uint32_t)amplitude << 8)) << cr_offset);
}

/**
 * @brief 禁用波形生成
 */
void f407_dac_disable_wave(f407_dac_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;

    uint32_t cr_offset = handle->config.channel * 16;

    /* 清除波形位 */
    DAC->CR &= ~(DAC_CR_WAVE1 << cr_offset);
}

/*===========================================================================*/
/*                              DMA支持                                       */
/*===========================================================================*/

/**
 * @brief 使能DAC DMA
 */
void f407_dac_enable_dma(f407_dac_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;

    if (handle->config.channel == F407_DAC_CH1)
    {
        DAC->CR |= DAC_CR_DMAEN1;
    }
    else
    {
        DAC->CR |= DAC_CR_DMAEN2;
    }
}

/**
 * @brief 禁用DAC DMA
 */
void f407_dac_disable_dma(f407_dac_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
        return;

    if (handle->config.channel == F407_DAC_CH1)
    {
        DAC->CR &= ~DAC_CR_DMAEN1;
    }
    else
    {
        DAC->CR &= ~DAC_CR_DMAEN2;
    }
}

/**
 * @brief 获取DAC数据寄存器地址 (用于DMA)
 */
volatile uint32_t *f407_dac_get_dhr_addr(f407_dac_channel_t channel, f407_dac_align_t align)
{
    if (channel == F407_DAC_CH1)
    {
        switch (align)
        {
        case F407_DAC_ALIGN_12B_R:
            return &DAC->DHR12R1;
        case F407_DAC_ALIGN_12B_L:
            return &DAC->DHR12L1;
        case F407_DAC_ALIGN_8B_R:
            return (volatile uint32_t *)&DAC->DHR8R1;
        default:
            return &DAC->DHR12R1;
        }
    }
    else
    {
        switch (align)
        {
        case F407_DAC_ALIGN_12B_R:
            return &DAC->DHR12R2;
        case F407_DAC_ALIGN_12B_L:
            return &DAC->DHR12L2;
        case F407_DAC_ALIGN_8B_R:
            return (volatile uint32_t *)&DAC->DHR8R2;
        default:
            return &DAC->DHR12R2;
        }
    }
}

/*===========================================================================*/
/*                              双通道同时输出                                 */
/*===========================================================================*/

/**
 * @brief 初始化DAC双通道
 */
int f407_dac_dual_init(f407_dac_handle_t *handle1, f407_dac_handle_t *handle2)
{
    int ret;

    ret = f407_dac_init_simple(handle1, F407_DAC_CH1);
    if (ret != 0)
        return ret;

    ret = f407_dac_init_simple(handle2, F407_DAC_CH2);
    if (ret != 0)
    {
        f407_dac_deinit(handle1);
        return ret;
    }

    /* 使能双通道 */
    f407_dac_enable(handle1);
    f407_dac_enable(handle2);

    return 0;
}

/**
 * @brief 同时设置双通道输出值
 */
void f407_dac_dual_set_value(uint16_t value1, uint16_t value2)
{
    if (value1 > F407_DAC_MAX_VALUE_12BIT)
        value1 = F407_DAC_MAX_VALUE_12BIT;
    if (value2 > F407_DAC_MAX_VALUE_12BIT)
        value2 = F407_DAC_MAX_VALUE_12BIT;

    /* 使用双通道数据寄存器同时写入 */
    DAC->DHR12RD = (uint32_t)value1 | ((uint32_t)value2 << 16);
}

/**
 * @brief 软件触发双通道同时转换
 */
void f407_dac_dual_software_trigger(void)
{
    DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1 | DAC_SWTRIGR_SWTRIG2;
}
