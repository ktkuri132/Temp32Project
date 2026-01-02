#include "driver.h"
#include "df_delay.h"

extern df_delay_t delay;

// 毫秒延时函数 (基于while循环，72MHz系统时钟) - 统一接口封装
static int delay_ms_unified(df_arg_t arg)
{
    uint32_t ms = arg.us32;
    uint32_t i, j;
    for (i = 0; i < ms; i++)
    {
        // 1ms的循环次数，根据72MHz调整
        // 经验值：约需要7200次循环实现1ms延时
        for (j = 0; j < 7200; j++)
        {
            __NOP(); // 空操作，防止编译器优化
        }
    }
    return 0;
}

// 原始接口（保持兼容性）
void delay_ms(uint32_t ms)
{
    delay_ms_unified(arg_u32(ms));
}

void __delay_ms(uint32_t ms)
{
    delay_ms_unified(arg_u32(ms));
}

// 微秒延时函数 (基于while循环，72MHz系统时钟) - 统一接口封装
static int delay_us_unified(df_arg_t arg)
{
    uint32_t us = arg.us32;
    uint32_t i, j;
    for (i = 0; i < us; i++)
    {
        // 1us的循环次数，根据72MHz调整
        // 经验值：约需要7-8次循环实现1us延时
        for (j = 0; j < 8; j++)
        {
            __NOP(); // 空操作，防止编译器优化
        }
    }
    return 0;
}

// 原始接口（保持兼容性）
void delay_us(uint32_t us)
{
    delay_us_unified(arg_u32(us));
}

df_delay_t delay = {
    .init_flag = true,
    .init = NULL,
    .ms = delay_ms_unified,
    .us = delay_us_unified};