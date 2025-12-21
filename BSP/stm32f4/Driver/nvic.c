#include "driver.h"

/**
 * STM32F4 NVIC驱动
 */
int nvic_init(dev_arg_t arg)
{
    (void)arg;
    // 设置中断优先级分组 (4位抢占优先级，0位响应优先级)
    NVIC_SetPriorityGrouping(3); // Group 4: 4 bits preemption, 0 bits subpriority

    // 设置USART1中断优先级
    NVIC_SetPriority(USART1_IRQn, NVIC_EncodePriority(3, 2, 0));

    // 使能USART1中断
    NVIC_EnableIRQ(USART1_IRQn);
    return 0;
}
