#ifndef __DRIVER_H
#define __DRIVER_H

#include <stm32f10x.h>
#include <stdio.h>

void NVIC_Init(void);
void Systick_Init(void);

void LED_Init(void);
void LED_Toggle(void);
void LED_On(void);
void LED_Off(void);

void USART1_Init(uint32_t BaudRate);
void USART1_SendChar(char ch);
void USART1_SendString(char *str);
uint8_t USART1_ReceiveChar(void *None,uint8_t *data);

void ADC1_Init(void);

void delay_ms(uint32_t ms);
void delay_us(uint32_t us);

#endif