/**
 * @file driver.h
 * @brief STM32F4 Driver 层公共头文件
 * @note 包含所有 Driver 模块的声明
 */

#ifndef __DRIVER_H
#define __DRIVER_H

#include <stm32f4xx.h>
#include <stdio.h>
#include <stdint.h>
#include <dev_frame.h>
#include <misc.h>

/*============================ 设备名称定义 ============================*/
#define DEBUG_UART_NAME "usart_debug"
#define ONBOARD_LED_NAME "led_onboard"
#define OLED_SH1106_NAME "oled_dp"
#define OLED_SSD1306_NAME "oled_dp"
#define ADC1_NAME "adc1"

/*============================ SysTick 模式定义 ============================*/
typedef enum
{
    SYSTICK_MODE_INTERRUPT = 0, /**< 中断模式 */
    SYSTICK_MODE_POLLING = 1    /**< 计数模式（轮询） */
} systick_mode_t;

/*============================ NVIC 接口 ============================*/
int nvic_init(df_arg_t arg);

/*============================ SysTick 接口 ============================*/
/* 中断模式 */
void Systick_Init_us(uint32_t interval_us);
void Systick_Init_ms(uint32_t interval_ms);
void Systick_Init(uint32_t ms);

/* 计数模式（精确延时） */
void Systick_Init_Polling(void);
void Systick_Delay_us(uint32_t us);
void Systick_Delay_ms(uint32_t ms);
systick_mode_t Systick_GetMode(void);

/*============================ LED 接口 ============================*/
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

/*============================ 延时接口 ============================*/
void delay_ms(uint32_t ms);
void delay_us(uint32_t us);

/*============================ I2C 设备接口 ============================*/
int sh1106_dev_init(df_arg_t arg);
int ssd1306_dev_init(df_arg_t arg);
uint8_t mpu6050_i2c_write(uint8_t addr, uint8_t reg, uint16_t length, uint8_t *data);
uint8_t mpu6050_i2c_read(uint8_t addr, uint8_t reg, uint16_t length, uint8_t *data);

#endif /* __DRIVER_H */
