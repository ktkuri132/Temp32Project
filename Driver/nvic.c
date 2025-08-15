#include "driver.h"
#include "misc.h"
void NVIC_Init(void)
{
    //设置中断优先级分组
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 2位抢占优先级，2位响应优先级
    // 设置优先级
    NVIC_SetPriority(USART1_IRQn, 2); // 设置中断优先级
    // NVIC_SetPriority(SysTick_IRQn, 0); // 设置SysTick中断优先级为最高
    // 使能中断
    NVIC_EnableIRQ(USART1_IRQn); // 使能USART1中断
    // NVIC_EnableIRQ(SysTick_IRQn); // 使能SysTick中断

}