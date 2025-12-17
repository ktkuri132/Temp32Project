#include <stm32f10x.h>
#include <shell/shell.h>
#include <driver.h>
#include "main.h"
#include <stddef.h>

void SysTick_CallBack(void)
{
    // 每1ms调用一次
    led.toggle(arg_null);
}

void USART1_IRQHandler(void)
{
    if (USART1->SR & USART_SR_RXNE) // 检查接收中断标志
    {
        BIE_UART(NULL,&Shell_Sysfpoint,&Shell,&env_vars,&STM32F103C8T6_Device); // 处理接收到的数据
        // 清除接收中断标志
        USART1->SR &= ~USART_SR_RXNE; // 清除RXNE标志
    }
}