#include "driver.h"
#include "df_adc.h"

extern At adc1;

/**
 * STM32F4 ADC1驱动
 * 默认使用 PA0 (ADC1_IN0)
 */

int adc1_init(dev_arg_t arg)
{
    (void)arg;

    // 1. 使能ADC1和GPIOA时钟
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    // 2. 配置PA0为模拟输入
    GPIOA->MODER |= (0x3 << (0 * 2)); // 模拟模式

    // 3. 配置ADC1
    // 关闭ADC以便配置
    ADC1->CR2 &= ~ADC_CR2_ADON;

    // 设置分辨率为12位
    ADC1->CR1 &= ~ADC_CR1_RES;

    // 单次转换模式
    ADC1->CR2 &= ~ADC_CR2_CONT;

    // 设置采样时间 (480周期，最高精度)
    ADC1->SMPR2 |= (0x7 << (0 * 3)); // 通道0

    // 设置转换序列，只有一个通道
    ADC1->SQR1 &= ~ADC_SQR1_L; // 1次转换
    ADC1->SQR3 = 0;            // 第一个转换是通道0

    // 4. 启动ADC
    ADC1->CR2 |= ADC_CR2_ADON;

    adc1.ADC_Init_Flag = true;
    return 0;
}

int adc1_enable(dev_arg_t arg)
{
    (void)arg;
    // 启动ADC1转换
    ADC1->CR2 |= ADC_CR2_SWSTART;
    return 0;
}

int adc1_disable(dev_arg_t arg)
{
    (void)arg;
    // 关闭ADC1
    ADC1->CR2 &= ~ADC_CR2_ADON;
    adc1.ADC_Init_Flag = false;
    return 0;
}

int adc1_get_value(dev_arg_t arg)
{
    if (!adc1.ADC_Init_Flag)
    {
        return -1;
    }

    // 启动转换
    ADC1->CR2 |= ADC_CR2_SWSTART;

    // 等待转换完成
    while (!(ADC1->SR & ADC_SR_EOC))
        ;

    // 读取转换结果
    arg.us32 = ADC1->DR;
    return 0;
}

At adc1 = {
    .ADC_Init_Flag = false,
    .ADC_Num = 1,
    .ADC_Name = ADC1_NAME,
    .init = adc1_init,
    .deinit = adc1_disable,
    .get_value = adc1_get_value};
