#include "driver.h"

void ADC1_Init(void)
{
    // 1. 使能ADC1时钟
    RCC->APB2ENR |= (1 << 9); // ADC1EN = 1

    // 2. 配置PA0为模拟输入
    RCC->APB2ENR |= (1 << 2); // IOPAEN = 1
    GPIOA->CRL &= ~(0xF << 0); // 清除PA0的配置
    GPIOA->CRL |= (0x4 << 0);  // 设置PA0为模拟输入模式

    // 3. 配置ADC1
    ADC1->CR1 = 0; // 无扫描模式，单通道转换
    ADC1->CR2 = (1 << 0) | (1 << 10); // 启用软件触发，启用连续转换模式
    ADC1->SMPR2 = (7 << 0); // 设置采样时间为239.5周期（最高精度）

    // 4. 启动ADC
    ADC1->CR2 |= (1 << 0); // ADON = 1，开启ADC
}