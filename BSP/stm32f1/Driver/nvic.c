/**
 * @file nvic.c
 * @brief STM32F1 NVIC中断控制器配置
 * @note 使用 CMSIS 标准接口
 */

#include "driver.h"
#include "misc.h"
#include <stm32f10x.h>

/**
 * @brief NVIC初始化
 * @note 配置中断优先级分组和各外设中断
 */
void NVIC_Init(void)
{
    /* 设置中断优先级分组: 2位抢占优先级，2位响应优先级 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    /* 配置USART1中断 */
    NVIC_SetPriority(USART1_IRQn, 2);
    NVIC_EnableIRQ(USART1_IRQn);

    /* SysTick中断已由系统自动配置 */
    // NVIC_SetPriority(SysTick_IRQn, 0);
    // NVIC_EnableIRQ(SysTick_IRQn);
}

/**
 * @brief NVIC初始化（Driver_Framework接口）
 * @param arg 参数（未使用）
 * @return 0成功
 */
int nvic_init(df_arg_t arg)
{
    (void)arg; /* 忽略参数 */
    NVIC_Init();
    return 0;
}