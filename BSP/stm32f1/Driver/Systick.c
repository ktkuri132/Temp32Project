#include "driver.h"
#include "df_log.h"

/**
 * STM32F1 SysTick驱动
 * 支持两种模式：
 *   1. 中断模式 - 定时产生中断
 *   2. 计数模式 - 仅计数，用于精确延时（不产生中断）
 */

static systick_mode_t g_systick_mode = SYSTICK_MODE_INTERRUPT;

/**
 * @brief 初始化SysTick定时器（中断模式，微秒级）
 * @param interval_us 中断间隔时间（微秒）
 * @note 最大间隔时间 = (2^24 - 1) / (SystemCoreClock / 1000000) 微秒
 *       对于168MHz时钟，最大约为 99.86ms
 */
void Systick_Init_us(uint32_t interval_us)
{
    uint32_t ticks = (SystemCoreClock / 1000000) * interval_us;

    // 检查是否超过24位计数器最大值
    if (ticks > 0xFFFFFF)
    {
        ticks = 0xFFFFFF;
    }

    g_systick_mode = SYSTICK_MODE_INTERRUPT;
    SysTick->LOAD = ticks - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk;
}

/**
 * @brief 初始化SysTick定时器（中断模式，毫秒级）
 * @param interval_ms 中断间隔时间（毫秒）
 * @note 最大间隔时间约为99ms（168MHz时钟）
 */
void Systick_Init_ms(uint32_t interval_ms)
{
    Systick_Init_us(interval_ms * 1000);
}

/**
 * @brief 初始化SysTick为计数模式（不产生中断）
 * @note 用于精确延时，配合 Systick_Delay_us/ms 使用
 */
void Systick_Init_Polling(void)
{
    g_systick_mode = SYSTICK_MODE_POLLING;
    SysTick->LOAD = 0xFFFFFF; // 最大值
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_ENABLE_Msk; // 不使能中断
}

/**
 * @brief 微秒级延时（阻塞）
 * @param us 延时微秒数
 * @note 需要先调用 Systick_Init_Polling() 或在中断模式下也可使用
 */
void Systick_Init(uint32_t ms)
{
    if (g_systick_mode == SYSTICK_MODE_POLLING)
    {
        Systick_Init_Polling();
    }
    else
    {
        Systick_Init_ms(ms); // 默认1ms
    }
}

/**
 * @brief 微秒级延时（阻塞）
 * @param us 延时微秒数
 * @note 需要先调用 Systick_Init_Polling() 或在中断模式下也可使用
 */
void Systick_Delay_us(uint32_t us)
{
    uint32_t ticks = (SystemCoreClock / 1000000) * us;
    uint32_t start = SysTick->VAL;
    uint32_t elapsed;

    while (1)
    {
        uint32_t current = SysTick->VAL;
        // SysTick 是向下计数的
        if (current <= start)
        {
            elapsed = start - current;
        }
        else
        {
            elapsed = start + (SysTick->LOAD + 1 - current);
        }
        if (elapsed >= ticks)
            break;
    }
}

/**
 * @brief 毫秒级延时（阻塞）
 * @param ms 延时毫秒数
 */
void Systick_Delay_ms(uint32_t ms)
{
    while (ms--)
    {
        Systick_Delay_us(1000);
    }
}

/**
 * @brief 获取当前 SysTick 模式
 * @return SYSTICK_MODE_INTERRUPT 或 SYSTICK_MODE_POLLING
 */
systick_mode_t Systick_GetMode(void)
{
    return g_systick_mode;
}

static uint64_t Systick_time;

uint32_t get_tick(void)
{
    if (g_systick_mode == SYSTICK_MODE_POLLING)
    {
        return SysTick->VAL;
    }
    else
    {
        return (uint32_t)(Systick_time);
    }
}

void SysTick_Handler(void)
{
    // 在此处调用需要在SysTick中断中执行的函数
    Systick_time++;
}

// DF_INIT_EXPORT(systick_init, DF_INIT_EXPORT_PREV);
