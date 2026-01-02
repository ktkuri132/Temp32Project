/**
 * @file nvic.c
 * @brief STM32F4 NVIC 中断控制器配置
 * @note 使用 CMSIS 标准接口
 */

#include "driver.h"

/*============================ 内部函数 ============================*/

/**
 * @brief NVIC 初始化
 * @note 配置中断优先级分组和各外设中断
 */
static void NVIC_Config(void)
{
    /* 设置中断优先级分组: 4位抢占优先级，0位响应优先级 */
    NVIC_SetPriorityGrouping(3); /* Group 4: 4 bits preemption, 0 bits subpriority */

    /* 配置 USART1 中断 */
    NVIC_SetPriority(USART1_IRQn, NVIC_EncodePriority(3, 2, 0));
    NVIC_EnableIRQ(USART1_IRQn);

    /* SysTick 中断已由系统自动配置 */
}

/*============================ 接口实现 ============================*/

/**
 * @brief NVIC 初始化（Driver_Framework 接口）
 * @param arg 参数（未使用）
 * @return 0 成功
 */
int nvic_init(df_arg_t arg)
{
    (void)arg;
    NVIC_Config();
    return 0;
}
