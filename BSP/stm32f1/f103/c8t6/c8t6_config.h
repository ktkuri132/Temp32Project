/**
 * @file c8t6_config.h
 * @brief STM32F103C8T6 芯片配置文件
 * @note C8T6: 64KB Flash, 20KB SRAM, LQFP48封装
 */

#ifndef __C8T6_CONFIG_H
#define __C8T6_CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif

/* ============== 芯片型号标识 ============== */
#define STM32F103C8T6 1
#define CHIP_PACKAGE "C8T6"
#define CHIP_MODEL "STM32F103C8T6"

/* ============== 内存配置 ============== */
#define FLASH_SIZE (64 * 1024) /* 64KB Flash */
#define SRAM_SIZE (20 * 1024)  /* 20KB SRAM */
#define FLASH_BASE_ADDR 0x08000000
#define SRAM_BASE_ADDR 0x20000000

/* ============== 时钟配置 ============== */
#define HSE_VALUE 8000000    /* 8MHz 外部晶振 */
#define HSI_VALUE 8000000    /* 8MHz 内部RC */
#define SYSCLK_FREQ 72000000 /* 系统时钟 72MHz */
#define HCLK_FREQ 72000000   /* AHB时钟 72MHz */
#define PCLK1_FREQ 36000000  /* APB1时钟 36MHz */
#define PCLK2_FREQ 72000000  /* APB2时钟 72MHz */

/* ============== GPIO配置 (LQFP48封装) ============== */
#define GPIO_PORT_COUNT 3 /* GPIOA, GPIOB, GPIOC (部分) */
#define GPIOA_AVAILABLE 1
#define GPIOB_AVAILABLE 1
#define GPIOC_AVAILABLE 1 /* PC13-PC15 */
#define GPIOD_AVAILABLE 0 /* 不可用 */

/* ============== 外设配置 ============== */
/* USART */
#define USART1_AVAILABLE 1
#define USART2_AVAILABLE 1
#define USART3_AVAILABLE 1

/* SPI */
#define SPI1_AVAILABLE 1
#define SPI2_AVAILABLE 1

/* I2C */
#define I2C1_AVAILABLE 1
#define I2C2_AVAILABLE 1

/* 定时器 */
#define TIM1_AVAILABLE 1
#define TIM2_AVAILABLE 1
#define TIM3_AVAILABLE 1
#define TIM4_AVAILABLE 1

/* ADC */
#define ADC1_AVAILABLE 1
#define ADC2_AVAILABLE 1
#define ADC_CHANNEL_COUNT 10 /* PA0-PA7, PB0-PB1 */

/* CAN */
#define CAN1_AVAILABLE 1

/* USB */
#define USB_AVAILABLE 1

/* ============== 默认引脚配置 ============== */
/* LED (板载通常连接PC13) */
#define LED_PORT F103_GPIOC
#define LED_PIN F103_PIN_13
#define LED_ACTIVE_LOW 1 /* 低电平点亮 */

/* 调试串口 (USART1: PA9-TX, PA10-RX) */
#define DEBUG_USART F103_USART1
#define DEBUG_USART_BAUD 115200

/* SPI Flash/传感器 (SPI1: PA5-SCK, PA6-MISO, PA7-MOSI) */
#define DEFAULT_SPI F103_SPI1

/* I2C 传感器 (I2C1: PB6-SCL, PB7-SDA) */
#define DEFAULT_I2C F103_I2C1

/* ============== 中断优先级配置 ============== */
#define NVIC_PRIORITY_GROUP NVIC_PriorityGroup_2
#define SYSTICK_PRIORITY 0
#define USART1_PRIORITY 1
#define USART2_PRIORITY 2
#define TIM2_PRIORITY 2
#define TIM3_PRIORITY 2

#ifdef __cplusplus
}
#endif

#endif /* __C8T6_CONFIG_H */
