#include "driver.h"
#include "df_adc.h"

extern At adc1 ;

int adc1_init(dev_arg_t arg) {
    (void)arg;  // 忽略参数
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
    return 0;    // 返回0表示成功
}

int adc1_enable(dev_arg_t arg) {
    (void)arg;  // 忽略参数
    // 启动ADC1转换
    ADC1->CR2 |= (1 << 30); // SWSTART = 1，启动转换
    return 0;    // 返回0表示成功
}

int adc1_disable(dev_arg_t arg) {
    (void)arg;  // 忽略参数
    // 停止ADC1转换
    ADC1->CR2 &= ~(1 << 30); // SWSTART = 0，停止转换
    // 失能时钟
    RCC->APB2ENR &= ~(1 << 9); // ADC1EN = 0
    return 0;    // 返回0表示成功
}

int adc1_get_value(dev_arg_t arg) {
    static int tmp = 0;
    if(!tmp) {
        if(!adc1.ADC_Init_Flag){
            return -1; // ADC未初始化，返回0
        }
    }
    // 等待转换完成
    while (!(ADC1->SR & (1 << 1)))
        ; // 等待EOC标志
    // 读取转换结果
    arg.us32 = ADC1->DR; // 获取ADC数值
    return 0; // 返回ADC数值
}

At adc1 = {
    .ADC_Init_Flag = false,
    .ADC_Num = 1,
    .ADC_Name = ADC1_NAME,
    .init = adc1_init,
    .deinit = adc1_disable,
    .get_value = adc1_get_value
};