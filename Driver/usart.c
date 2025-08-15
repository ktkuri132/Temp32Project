#include "driver.h"
#include <stdarg.h>
void USART1_Init(uint32_t BaudRate)
{
    // 使能GPIOA和USART1时钟
    RCC->APB2ENR |= (1 << 2); // IOPAEN = 1
    RCC->APB2ENR |= (1 << 14); // USART1EN = 1

    // 配置PA9为复用推挽输出（TX）
    GPIOA->CRH &= ~(0xF << 4); // 清除PA9的配置
    GPIOA->CRH |= (0xB << 4);  // 设置PA9为复用推挽输出

    // 配置PA10为浮空输入（RX）
    GPIOA->CRH &= ~(0xF << 8); // 清除PA10的配置
    GPIOA->CRH |= (0x4 << 8);  // 设置PA10为浮空输入

    // 配置USART1
    USART1->BRR = SystemCoreClock / BaudRate; // 设置波特率
    USART1->CR1 = (1 << 13) | (1 << 3) | (1 << 2); // 启用USART, TX, RX

    // 开启中断
    USART1->CR1 |= (1 << 5); // 启用RXNE中断

}

int __io_putchar(int ch){
    USART1_SendChar(ch); // 发送字符到USART1
    return ch; // 返回发送的字符
}

// 串口发送单个字符
void USART1_SendChar(char ch)
{
    while(!(USART1->SR & (1 << 7))); // 等待TXE标志
    USART1->DR = ch;
}

// 串口发送字符串
void USART1_SendString(char *str)
{
    while(*str)
    {
        USART1_SendChar(*str++);
    }
}


uint8_t USART1_ReceiveChar(void *None,uint8_t *data){
    (void)None; // 避免未使用参数警告
    while(!(USART1->SR & (1 << 5))); // 等待RXNE标志
    *data = USART1->DR; // 返回接收到的字符
    return *data; // 返回接收到的字符
}

int printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[256]; // 缓冲区大小可以根据需要调整
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    USART1_SendString(buffer); // 发送格式化字符串到USART1
    return len; // 返回发送的字符长度
}
