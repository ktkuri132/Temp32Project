/**
 * @file f103_adc.c
 * @brief STM32F103 系列 ADC 驱动实现
 */

#include "f103_adc.h"
#include "f103_gpio.h"

#define ADC_TIMEOUT 10000

static ADC_TypeDef *const adc_base_table[F103_ADC_MAX] = {
    ADC1, ADC2};

/* ADC通道对应的GPIO (通道0-7: PA0-PA7, 通道8-9: PB0-PB1, 通道10-15: PC0-PC5) */
static const struct
{
    f103_gpio_port_t port;
    f103_gpio_pin_t pin;
} adc_channel_gpio[] = {
    {F103_GPIOA, F103_PIN_0}, /* CH0 */
    {F103_GPIOA, F103_PIN_1}, /* CH1 */
    {F103_GPIOA, F103_PIN_2}, /* CH2 */
    {F103_GPIOA, F103_PIN_3}, /* CH3 */
    {F103_GPIOA, F103_PIN_4}, /* CH4 */
    {F103_GPIOA, F103_PIN_5}, /* CH5 */
    {F103_GPIOA, F103_PIN_6}, /* CH6 */
    {F103_GPIOA, F103_PIN_7}, /* CH7 */
    {F103_GPIOB, F103_PIN_0}, /* CH8 */
    {F103_GPIOB, F103_PIN_1}, /* CH9 */
    {F103_GPIOC, F103_PIN_0}, /* CH10 */
    {F103_GPIOC, F103_PIN_1}, /* CH11 */
    {F103_GPIOC, F103_PIN_2}, /* CH12 */
    {F103_GPIOC, F103_PIN_3}, /* CH13 */
    {F103_GPIOC, F103_PIN_4}, /* CH14 */
    {F103_GPIOC, F103_PIN_5}, /* CH15 */
};

static void f103_adc_clk_enable(f103_adc_port_t port)
{
    /* 使能ADC时钟 */
    switch (port)
    {
    case F103_ADC1:
        RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
        break;
    case F103_ADC2:
        RCC->APB2ENR |= RCC_APB2ENR_ADC2EN;
        break;
    default:
        break;
    }

    /* ADC预分频: PCLK2/6 = 12MHz (最大14MHz) */
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_ADCPRE) | RCC_CFGR_ADCPRE_DIV6;
}

static void f103_adc_gpio_init(f103_adc_channel_t channel)
{
    if (channel < 16)
    {
        f103_gpio_init_quick(adc_channel_gpio[channel].port,
                             adc_channel_gpio[channel].pin,
                             F103_GPIO_MODE_AIN,
                             F103_GPIO_SPEED_2MHZ);
    }
}

int f103_adc_init(const f103_adc_config_t *config)
{
    if (config == NULL || config->port >= F103_ADC_MAX)
        return -1;

    ADC_TypeDef *adc = adc_base_table[config->port];

    f103_adc_clk_enable(config->port);

    /* 配置ADC模式 */
    adc->CR1 = 0;
    adc->CR2 = 0;

    if (config->scan)
    {
        adc->CR1 |= ADC_CR1_SCAN;
    }

    if (config->continuous)
    {
        adc->CR2 |= ADC_CR2_CONT;
    }

    /* 软件触发 */
    adc->CR2 |= ADC_CR2_EXTSEL;

    /* 使能ADC */
    adc->CR2 |= ADC_CR2_ADON;

    /* 等待ADC稳定 */
    for (volatile int i = 0; i < 10000; i++)
        ;

    /* 校准ADC */
    f103_adc_calibrate(config->port);

    return 0;
}

int f103_adc_init_quick(f103_adc_port_t port)
{
    f103_adc_config_t config = {
        .port = port,
        .continuous = false,
        .scan = false,
        .sample_time = F103_ADC_SAMPLETIME_55_5};
    return f103_adc_init(&config);
}

int f103_adc_config_channel(f103_adc_port_t port, f103_adc_channel_t channel,
                            f103_adc_sampletime_t sample_time)
{
    if (port >= F103_ADC_MAX || channel >= F103_ADC_CH_MAX)
        return -1;

    ADC_TypeDef *adc = adc_base_table[port];

    /* 初始化对应GPIO */
    f103_adc_gpio_init(channel);

    /* 设置采样时间 */
    if (channel < 10)
    {
        uint32_t shift = channel * 3;
        adc->SMPR2 = (adc->SMPR2 & ~(0x07 << shift)) | (sample_time << shift);
    }
    else
    {
        uint32_t shift = (channel - 10) * 3;
        adc->SMPR1 = (adc->SMPR1 & ~(0x07 << shift)) | (sample_time << shift);
    }

    return 0;
}

uint16_t f103_adc_read(f103_adc_port_t port, f103_adc_channel_t channel)
{
    if (port >= F103_ADC_MAX || channel >= F103_ADC_CH_MAX)
        return 0;

    ADC_TypeDef *adc = adc_base_table[port];

    /* 初始化GPIO */
    f103_adc_gpio_init(channel);

    /* 设置通道序列 */
    adc->SQR1 = 0; /* 1个转换 */
    adc->SQR3 = channel;

    /* 设置采样时间 */
    f103_adc_config_channel(port, channel, F103_ADC_SAMPLETIME_55_5);

    /* 启动转换 */
    adc->CR2 |= ADC_CR2_ADON;

    /* 等待转换完成 */
    if (f103_adc_wait(port) != 0)
        return 0;

    return f103_adc_get_value(port);
}

uint32_t f103_adc_read_voltage(f103_adc_port_t port, f103_adc_channel_t channel,
                               uint32_t vref)
{
    uint16_t adc_value = f103_adc_read(port, channel);
    return (uint32_t)adc_value * vref / 4095;
}

void f103_adc_start(f103_adc_port_t port)
{
    if (port < F103_ADC_MAX)
    {
        ADC_TypeDef *adc = adc_base_table[port];
        adc->CR2 |= ADC_CR2_ADON;
    }
}

int f103_adc_wait(f103_adc_port_t port)
{
    if (port >= F103_ADC_MAX)
        return -1;

    ADC_TypeDef *adc = adc_base_table[port];
    uint32_t timeout = ADC_TIMEOUT;

    while (!(adc->SR & ADC_SR_EOC) && timeout--)
        ;

    return timeout ? 0 : -1;
}

uint16_t f103_adc_get_value(f103_adc_port_t port)
{
    if (port >= F103_ADC_MAX)
        return 0;

    return adc_base_table[port]->DR & 0x0FFF;
}

void f103_adc_enable_temp_sensor(void)
{
    ADC1->CR2 |= ADC_CR2_TSVREFE;
}

int16_t f103_adc_read_temperature(void)
{
    /* 使能温度传感器 */
    f103_adc_enable_temp_sensor();

    /* 读取内部温度传感器通道(CH16) */
    uint16_t adc_value = f103_adc_read(F103_ADC1, F103_ADC_CH16);

    /* 计算温度: T = (V25 - Vsense) / Avg_Slope + 25
     * V25 = 1.43V, Avg_Slope = 4.3mV/°C
     * Vsense = adc_value * 3300 / 4095 (mV)
     */
    int32_t vsense = (int32_t)adc_value * 3300 / 4095;
    int32_t temp = (1430 - vsense) * 10 / 43 + 250; /* 温度*10 */

    return (int16_t)temp;
}

void f103_adc_calibrate(f103_adc_port_t port)
{
    if (port >= F103_ADC_MAX)
        return;

    ADC_TypeDef *adc = adc_base_table[port];

    /* 复位校准 */
    adc->CR2 |= ADC_CR2_RSTCAL;
    while (adc->CR2 & ADC_CR2_RSTCAL)
        ;

    /* 开始校准 */
    adc->CR2 |= ADC_CR2_CAL;
    while (adc->CR2 & ADC_CR2_CAL)
        ;
}

int f103_adc_dma_init(f103_adc_port_t port, const f103_adc_channel_t *channels,
                      uint8_t channel_count, uint16_t *buffer)
{
    if (port != F103_ADC1 || channels == NULL || buffer == NULL || channel_count == 0)
        return -1;

    ADC_TypeDef *adc = adc_base_table[port];

    /* 使能DMA时钟 */
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;

    /* 配置通道 */
    for (uint8_t i = 0; i < channel_count; i++)
    {
        f103_adc_gpio_init(channels[i]);
        f103_adc_config_channel(port, channels[i], F103_ADC_SAMPLETIME_55_5);
    }

    /* 设置规则通道序列 */
    adc->SQR1 = ((channel_count - 1) << 20);
    adc->SQR3 = 0;
    for (uint8_t i = 0; i < channel_count && i < 6; i++)
    {
        adc->SQR3 |= (channels[i] << (i * 5));
    }

    /* 配置DMA通道1 (ADC1) */
    DMA1_Channel1->CCR = 0;
    DMA1_Channel1->CPAR = (uint32_t)&adc->DR;
    DMA1_Channel1->CMAR = (uint32_t)buffer;
    DMA1_Channel1->CNDTR = channel_count;
    DMA1_Channel1->CCR = DMA_CCR1_MINC |    /* 内存地址递增 */
                         DMA_CCR1_PSIZE_0 | /* 外设16位 */
                         DMA_CCR1_MSIZE_0 | /* 内存16位 */
                         DMA_CCR1_CIRC;     /* 循环模式 */

    /* 使能DMA */
    DMA1_Channel1->CCR |= DMA_CCR1_EN;

    /* 配置ADC为扫描+连续+DMA模式 */
    adc->CR1 |= ADC_CR1_SCAN;
    adc->CR2 |= ADC_CR2_CONT | ADC_CR2_DMA;

    return 0;
}

void f103_adc_dma_start(f103_adc_port_t port)
{
    if (port == F103_ADC1)
    {
        ADC1->CR2 |= ADC_CR2_ADON;
    }
}
