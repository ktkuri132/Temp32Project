/**
 * @file driver.h
 * @brief STM32F1 驱动层头文件
 * @note 对接 Driver_Framework 接口，使用 f103 底层驱动
 */

#ifndef __DRIVER_H
#define __DRIVER_H

#include <stm32f10x.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <dev_frame.h>

/* 包含 f103 底层驱动 */
#include "f103_gpio.h"
#include "f103_usart.h"
#include "f103_adc.h"

#include <i2c/df_iic.h>

extern df_iic_t i2c1_bus; /* 外部软件I2C总线 */

/*============================ 设备名称定义 ============================*/
#define DEBUG_UART_NAME "usart_debug"
#define ONBOARD_LED_NAME "led_onboard"
#define OLED_SH1106_NAME "oled_dp"
#define OLED_SSD1306_NAME "oled_dp"
#define ADC1_NAME "adc1"

/*============================ NVIC 接口 ============================*/
void NVIC_Init(void);
int nvic_init(df_arg_t arg);

/*============================ SysTick 接口 ============================*/
typedef enum
{
    SYSTICK_MODE_INTERRUPT = 0, /* 中断模式 */
    SYSTICK_MODE_POLLING = 1    /* 计数模式（轮询）*/
} systick_mode_t;

void Systick_Init_us(uint32_t interval_us);
void Systick_Init_ms(uint32_t interval_ms);
void Systick_Init(uint32_t ms);
void Systick_Init_Polling(void);
void Systick_Delay_us(uint32_t us);
void Systick_Delay_ms(uint32_t ms);
systick_mode_t Systick_GetMode(void);
int systick_init(df_arg_t arg);

/*============================ 延时接口 ============================*/
void delay_ms(uint32_t ms);
void delay_us(uint32_t us);

/*============================ LED/GPIO 接口 ============================*/
int led_init(df_arg_t arg);
int led_on(df_arg_t arg);
int led_off(df_arg_t arg);
int led_toggle(df_arg_t arg);

/*============================ USART 接口 ============================*/
int usart1_init(df_arg_t arg);
int usart1_deinit(df_arg_t arg);
int usart1_send(df_arg_t arg);
int usart1_receive(df_arg_t arg);

/*============================ ADC 接口 ============================*/
int adc1_init(df_arg_t arg);
int adc1_deinit(df_arg_t arg);
int adc1_get_value(df_arg_t arg);

/*============================ 显示设备接口 ============================*/
int sh1106_dev_init(df_arg_t arg);
int ssd1306_dev_init(df_arg_t arg);

#endif /* __DRIVER_H */
