#include "driver.h"

/**
 * STM32F4 SysTick驱动
 */

void Systick_Init(void)
{
    // 配置SysTick定时器 - 1ms中断
    SysTick->LOAD = SystemCoreClock / 1000 - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk;
}

int systick_init(dev_arg_t arg)
{
    (void)arg;
    Systick_Init();
    return 0;
}
