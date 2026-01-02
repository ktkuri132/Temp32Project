#include "driver.h"
#include <stdarg.h>
#include "df_uart.h"
#include <df_init.h>
#include <df_log.h>
#include "main.h"

int usart1_init();
int usart1_stop(dev_arg_t arg);
void usart1_send(const char *data);
int usart1_receive(dev_arg_t arg);

Ut debug = {
    .UART_Init_Flag = false,
    .UART_Num = 1,
    .UART_Name = DEBUG_UART_NAME,
    .BaudRate = 250000,
    .init = usart1_init,
    .deinit = usart1_stop,
    .send = NULL,
    .printf = printf,
    .receive = usart1_receive,
    .send_withDMA = NULL,
    .receive_withDMA = NULL};

/**
 * STM32F4 USART1驱动
 * 默认引脚: PA9(TX), PA10(RX)
 * 时钟: APB2 (最高84MHz)
 */

void USART1_Init(uint32_t BaudRate)
{
    // 1. 使能GPIOA和USART1时钟
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    // 2. 配置PA9为复用功能（TX）
    GPIOA->MODER &= ~(0x3 << (9 * 2));
    GPIOA->MODER |= (0x2 << (9 * 2));   // 复用功能
    GPIOA->OTYPER &= ~(1 << 9);         // 推挽输出
    GPIOA->OSPEEDR |= (0x3 << (9 * 2)); // 高速
    GPIOA->PUPDR &= ~(0x3 << (9 * 2));
    GPIOA->PUPDR |= (0x1 << (9 * 2)); // 上拉
    GPIOA->AFR[1] &= ~(0xF << ((9 - 8) * 4));
    GPIOA->AFR[1] |= (0x7 << ((9 - 8) * 4)); // AF7 = USART1

    // 3. 配置PA10为复用功能（RX）
    GPIOA->MODER &= ~(0x3 << (10 * 2));
    GPIOA->MODER |= (0x2 << (10 * 2)); // 复用功能
    GPIOA->PUPDR &= ~(0x3 << (10 * 2));
    GPIOA->PUPDR |= (0x1 << (10 * 2)); // 上拉
    GPIOA->AFR[1] &= ~(0xF << ((10 - 8) * 4));
    GPIOA->AFR[1] |= (0x7 << ((10 - 8) * 4)); // AF7 = USART1

    // 4. 配置USART1
    // APB2时钟频率，假设为84MHz
    uint32_t apb2_clk = SystemCoreClock / 2; // APB2 = HCLK/2 (通常配置)
    USART1->BRR = apb2_clk / BaudRate;
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE; // 启用USART, TX, RX

    // 5. 开启接收中断
    USART1->CR1 |= USART_CR1_RXNEIE;
}

int __io_putchar(int ch)
{
    USART1_SendChar(ch);
    return ch;
}

void USART1_SendChar(char ch)
{
    while (!(USART1->SR & USART_SR_TXE))
        ;
    USART1->DR = ch;
}

void USART1_SendString(char *str)
{
    while (*str)
    {
        USART1_SendChar(*str++);
    }
}

uint8_t USART1_ReceiveChar(void *None, uint8_t *data)
{
    (void)None;
    while (!(USART1->SR & USART_SR_RXNE))
        ;
    *data = USART1->DR;
    return *data;
}

#include <lcd/df_lcd.h>

#ifndef __clang__

int printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[50];
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    USART1_SendString(buffer);
    // LCD_TerminalOut(&lcd_sh1106, (uint8_t*)buffer);
    return len;
}

#else

int fputc(int ch, FILE *f)
{
//    while (!(USART1->SR & USART_SR_TXE))
//        ;
//    USART1->DR = (uint8_t)ch;
    LCD_TerminalOut(&lcd_sh1106, (uint8_t*)&ch);
    return ch;
}

#endif


int usart1_init(){
    USART1_Init(250000);
    debug.UART_Init_Flag = true;
    g_log_config.output_func = usart1_send;
    LOG_I("USART1", "USART1 initialized with baud rate %d", debug.BaudRate);
    log_flush();
    return 0;
}

DF_INIT_EXPORT(usart1_init,DF_INIT_EXPORT_PREV);


void usart1_send(const char *data)
{
    USART1_SendString((char *)data);
}

int usart1_receive(dev_arg_t arg)
{
    uint8_t *data = (uint8_t *)arg.ptr;
    if (data == NULL)
    {
        return -1;
    }
    *data = USART1_ReceiveChar(NULL, data);
    return 0;
}

int usart1_start(dev_arg_t arg)
{
    (void)arg;
    USART1->CR1 |= USART_CR1_UE;
    return 0;
}

int usart1_stop(dev_arg_t arg)
{
    (void)arg;
    USART1->CR1 &= ~USART_CR1_UE;
    return 0;
}
