/**
 * @file delay.c
 * @brief STM32F4 延时驱动
 * @note 基于循环延时，适用于 168MHz 系统时钟
 * @note 实现 df_delay.h 接口
 */

#include "driver.h"
#include "df_delay.h"

/*============================ 前向声明 ============================*/
static int delay_ms_unified(df_arg_t arg);
static int delay_us_unified(df_arg_t arg);

/*============================ 延时设备实例 ============================*/
df_delay_t delay = {
    .init_flag = true,
    .init = NULL,
    .ms = delay_ms_unified,
    .us = delay_us_unified};

/*============================ 接口实现 ============================*/

/**
 * @brief 毫秒延时函数
 * @note 基于 while 循环，168MHz 系统时钟
 */
static int delay_ms_unified(df_arg_t arg)
{
    uint32_t ms = arg.us32;
    uint32_t i, j;
    for (i = 0; i < ms; i++)
    {
        /* 1ms 的循环次数，根据 168MHz 调整
         * 经验值：约需要 16800 次循环实现 1ms 延时 */
        for (j = 0; j < 16800; j++)
        {
            __NOP();
        }
    }
    return 0;
}

/**
 * @brief 微秒延时函数
 * @note 基于 while 循环，168MHz 系统时钟
 */
static int delay_us_unified(df_arg_t arg)
{
    uint32_t us = arg.us32;
    uint32_t i, j;
    for (i = 0; i < us; i++)
    {
        /* 1us 的循环次数，根据 168MHz 调整
         * 经验值：约需要 17 次循环实现 1us 延时 */
        for (j = 0; j < 17; j++)
        {
            __NOP();
        }
    }
    return 0;
}

/*============================ 兼容接口 ============================*/

/**
 * @brief 毫秒延时 (原始接口)
 */
void delay_ms(uint32_t ms)
{
    delay_ms_unified(arg_u32(ms));
}

/**
 * @brief 毫秒延时 (别名)
 */
void __delay_ms(uint32_t ms)
{
    delay_ms_unified(arg_u32(ms));
}

/**
 * @brief 微秒延时 (原始接口)
 */
void delay_us(uint32_t us)
{
    delay_us_unified(arg_u32(us));
}
