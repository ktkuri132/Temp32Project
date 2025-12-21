#include "driver.h"
#include "df_delay.h"

extern Dt delay;

/**
 * STM32F4 延时驱动
 * 基于循环延时，适用于168MHz系统时钟
 */

// 毫秒延时函数 (基于while循环，168MHz系统时钟)
void delay_ms(uint32_t ms)
{
    uint32_t i, j;
    for (i = 0; i < ms; i++)
    {
        // 1ms的循环次数，根据168MHz调整
        // 经验值：约需要16800次循环实现1ms延时
        for (j = 0; j < 16800; j++)
        {
            __NOP();
        }
    }
}

void __delay_ms(uint32_t ms)
{
    uint32_t i, j;
    for (i = 0; i < ms; i++)
    {
        for (j = 0; j < 16800; j++)
        {
            __NOP();
        }
    }
}

// 微秒延时函数 (基于while循环，168MHz系统时钟)
void delay_us(uint32_t us)
{
    uint32_t i, j;
    for (i = 0; i < us; i++)
    {
        // 1us的循环次数，根据168MHz调整
        // 经验值：约需要17次循环实现1us延时
        for (j = 0; j < 17; j++)
        {
            __NOP();
        }
    }
}

Dt delay = {
    .Delay_Init_Flag = true,
    .init = NULL,
    .ms = delay_ms,
    .us = delay_us};
