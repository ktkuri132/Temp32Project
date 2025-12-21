#ifndef __DRIVER_H
#define __DRIVER_H

#include <stm32f4xx.h>
#include <stdio.h>
#include <stdint.h>
#include <dev_frame.h>
#include <misc.h>

#define DEBUG_UART_NAME "usart_debug"
#define ONBOARD_LED_NAME "led_onboard"
#define OLED_SH1106_NAME "oled_dp"
#define OLED_SSD1306_NAME "oled_dp"
#define ADC1_NAME "adc1"

int nvic_init(dev_arg_t arg);

void Systick_Init(void);
int systick_init(dev_arg_t arg);

int led_init(dev_arg_t arg);
int led_on(dev_arg_t arg);
int led_off(dev_arg_t arg);

void USART1_Init(uint32_t BaudRate);
void USART1_SendChar(char ch);
void USART1_SendString(char *str);
uint8_t USART1_ReceiveChar(void *None, uint8_t *data);
int usart1_init(dev_arg_t arg);
int usart1_start(dev_arg_t arg);
int usart1_stop(dev_arg_t arg);

void ADC1_Init(void);
int adc1_init(dev_arg_t arg);
int adc1_enable(dev_arg_t arg);
int adc1_disable(dev_arg_t arg);

void delay_ms(uint32_t ms);
void delay_us(uint32_t us);

int sh1106_dev_init(dev_arg_t arg);
void SSD1306_SetPixel(uint16_t x, uint16_t y, uint32_t color);
int ssd1306_dev_init(dev_arg_t arg);

#endif
