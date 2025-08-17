#include "driver.h"

void Systick_Init(void)
{
    // 配置SysTick定时器
    SysTick->LOAD = SystemCoreClock / 1000 - 1; // 1ms中断
    SysTick->VAL = 0; // 清除计数器
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk; // 启用SysTick中断和计数器
}

int systick_init(dev_arg_t arg){
    (void)arg;  // 忽略参数
    Systick_Init();
    return 0;
}