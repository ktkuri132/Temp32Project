/**
 * @file irq.c
 * @brief STM32F1 中断服务函数
 * @note 使用 f103 驱动接口
 */

#include "driver.h"
#include "f103_usart.h"
#include <shell/df_shell.h>
#include "main.h"
#include <stddef.h>
#include <stm32f10x.h>

/**
 * @brief SysTick 中断回调函数
 * @note 每1ms调用一次（当SysTick配置为1ms中断时）
 */
void SysTick_CallBack(void)
{
    // 示例：LED翻转
    // led.toggle(arg_null);
}

/**
 * @brief USART1 接收中断处理函数
 */
void USART1_IRQHandler(void)
{
    // 使用 f103_usart 接口检查数据是否可读
    if (f103_usart_available(F103_USART1))
    {
        // 处理Shell命令
        BIE_UART(0, &Shell_Sysfpoint, &Shell, env_vars, &STM32F103C8T6_Device);

        // 清除接收中断标志（直接访问寄存器以保证中断处理速度）
        USART1->SR &= ~USART_SR_RXNE;
    }
}