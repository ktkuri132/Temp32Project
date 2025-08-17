#ifndef __DRIVER_H
#define __DRIVER_H

#include <stm32f10x.h>
#include <stdio.h>
#include <dev_model_core.h>

void NVIC_Init(void);
int nvic_init(dev_arg_t arg);

void Systick_Init(void);
int systick_init(dev_arg_t arg);

void LED_Init(void);
void LED_Toggle(void);
void LED_On(void);
void LED_Off(void);
int led_init(dev_arg_t arg);
int led_enable(dev_arg_t arg);
int led_disable(dev_arg_t arg);

void USART1_Init(uint32_t BaudRate);
void USART1_SendChar(char ch);
void USART1_SendString(char *str);
uint8_t USART1_ReceiveChar(void *None,uint8_t *data);
int usart1_init(dev_arg_t arg);
int usart1_start(dev_arg_t arg);
int usart1_stop(dev_arg_t arg);

void ADC1_Init(void);
int adc1_init(dev_arg_t arg);
int adc1_enable(dev_arg_t arg);
int adc1_disable(dev_arg_t arg);

void delay_ms(uint32_t ms);
void delay_us(uint32_t us);

#endif