/**
 * @file vet6_bsp.h
 * @brief STM32F407VET6 板级支持包统一头文件
 * @details 包含所有VET6板级驱动的头文件，便于用户使用
 *
 * @note 使用方法:
 *       只需在应用程序中包含此文件即可使用所有VET6板级驱动:
 *       #include "vet6_bsp.h"
 *
 * @section MCU特性
 * - MCU: STM32F407VET6
 * - Core: ARM Cortex-M4
 * - Flash: 512KB
 * - SRAM: 192KB (128KB + 64KB CCM)
 * - 封装: LQFP100
 * - GPIO: 82个可用IO (PA0-PA15, PB0-PB15, PC0-PC15, PD0-PD15, PE0-PE15)
 *
 * @section 外设资源
 * - GPIO: 5个端口 (GPIOA-GPIOE)
 * - TIM: 14个定时器 (TIM1-TIM14)
 * - SPI: 3个 (SPI1-SPI3)
 * - I2C: 3个 (I2C1-I2C3)
 * - USART: 6个 (USART1-USART3, UART4-UART5, USART6)
 * - ADC: 3个 (ADC1-ADC3), 16个外部通道+3个内部通道
 * - DAC: 2通道
 *
 * @section 时钟配置
 * - SystemCoreClock: 168MHz
 * - APB1: 42MHz (TIMx2 = 84MHz)
 * - APB2: 84MHz (TIMx2 = 168MHz)
 * - ADC预分频: /4 = 21MHz
 */

#ifndef __VET6_BSP_H
#define __VET6_BSP_H

#ifdef __cplusplus
extern "C"
{
#endif

/*===========================================================================*/
/*                              核心驱动                                      */
/*===========================================================================*/

/* GPIO驱动 - 所有其他驱动的基础 */
#include "vet6_gpio.h"

/*===========================================================================*/
/*                              通信接口                                      */
/*===========================================================================*/

/* SPI驱动 - 支持SPI1-SPI3 */
#include "vet6_spi.h"

/* I2C驱动 - 支持硬件I2C1-I2C3和软件I2C */
#include "vet6_i2c.h"

/* USART驱动 - 支持USART1-USART6 */
#include "vet6_usart.h"

/*===========================================================================*/
/*                              定时器                                        */
/*===========================================================================*/

/* 定时器驱动 - 支持TIM1-TIM14 */
#include "vet6_tim.h"

/*===========================================================================*/
/*                              模拟外设                                      */
/*===========================================================================*/

/* ADC驱动 - 支持ADC1-ADC3 */
#include "vet6_adc.h"

/* DAC驱动 - 支持DAC_CH1/CH2 */
#include "vet6_dac.h"

    /*===========================================================================*/
    /*                              版本信息                                      */
    /*===========================================================================*/

#define VET6_BSP_VERSION_MAJOR 1
#define VET6_BSP_VERSION_MINOR 0
#define VET6_BSP_VERSION_PATCH 0
#define VET6_BSP_VERSION ((VET6_BSP_VERSION_MAJOR << 16) | \
                          (VET6_BSP_VERSION_MINOR << 8) |  \
                          VET6_BSP_VERSION_PATCH)
#define VET6_BSP_VERSION_STRING "1.0.0"

    /*===========================================================================*/
    /*                              芯片信息                                      */
    /*===========================================================================*/

#define VET6_MCU_NAME "STM32F407VET6"
#define VET6_CORE "ARM Cortex-M4"
#define VET6_FLASH_SIZE (512 * 1024) /* 512KB */
#define VET6_SRAM_SIZE (128 * 1024)  /* 128KB Main SRAM */
#define VET6_CCMRAM_SIZE (64 * 1024) /* 64KB CCM RAM */
#define VET6_PACKAGE "LQFP100"
#define VET6_GPIO_PORTS 5       /* GPIOA-GPIOE */
#define VET6_GPIO_PINS_TOTAL 80 /* 5 * 16 */

    /*===========================================================================*/
    /*                              时钟信息                                      */
    /*===========================================================================*/

#define VET6_SYSCLK_FREQ 168000000UL   /* 168 MHz */
#define VET6_HCLK_FREQ 168000000UL     /* AHB = 168 MHz */
#define VET6_APB1_FREQ 42000000UL      /* APB1 = 42 MHz */
#define VET6_APB2_FREQ 84000000UL      /* APB2 = 84 MHz */
#define VET6_APB1_TIM_FREQ 84000000UL  /* APB1 Timer = 84 MHz */
#define VET6_APB2_TIM_FREQ 168000000UL /* APB2 Timer = 168 MHz */

/*===========================================================================*/
/*                              典型应用引脚定义                               */
/*===========================================================================*/

/**
 * @brief LED引脚定义 (根据实际硬件修改)
 */
#ifndef VET6_LED1_PIN
#define VET6_LED1_PIN VET6_PE0
#endif

#ifndef VET6_LED2_PIN
#define VET6_LED2_PIN VET6_PE1
#endif

/**
 * @brief 按键引脚定义 (根据实际硬件修改)
 */
#ifndef VET6_KEY1_PIN
#define VET6_KEY1_PIN VET6_PA0
#endif

/**
 * @brief 调试串口定义
 */
#ifndef VET6_DEBUG_USART
#define VET6_DEBUG_USART VET6_USART1
#endif

/*===========================================================================*/
/*                              便捷宏定义                                     */
/*===========================================================================*/

/**
 * @brief LED控制宏
 */
#define VET6_LED1_ON() vet6_gpio_reset(VET6_LED1_PIN)
#define VET6_LED1_OFF() vet6_gpio_set(VET6_LED1_PIN)
#define VET6_LED1_TOGGLE() vet6_gpio_toggle(VET6_LED1_PIN)

#define VET6_LED2_ON() vet6_gpio_reset(VET6_LED2_PIN)
#define VET6_LED2_OFF() vet6_gpio_set(VET6_LED2_PIN)
#define VET6_LED2_TOGGLE() vet6_gpio_toggle(VET6_LED2_PIN)

/**
 * @brief 按键读取宏
 */
#define VET6_KEY1_READ() vet6_gpio_read(VET6_KEY1_PIN)

    /*===========================================================================*/
    /*                              初始化函数                                     */
    /*===========================================================================*/

    /**
     * @brief 初始化所有GPIO时钟
     */
    static inline void vet6_bsp_init_gpio_clocks(void)
    {
        vet6_gpio_clk_enable(VET6_GPIOA);
        vet6_gpio_clk_enable(VET6_GPIOB);
        vet6_gpio_clk_enable(VET6_GPIOC);
        vet6_gpio_clk_enable(VET6_GPIOD);
        vet6_gpio_clk_enable(VET6_GPIOE);
    }

    /**
     * @brief 初始化LED引脚
     * @note 假设LED低电平点亮
     */
    static inline void vet6_bsp_init_leds(void)
    {
        vet6_gpio_config_output(VET6_LED1_PIN, VET6_GPIO_SPEED_LOW, VET6_GPIO_PUSH_PULL, VET6_GPIO_NOPULL);
        vet6_gpio_config_output(VET6_LED2_PIN, VET6_GPIO_SPEED_LOW, VET6_GPIO_PUSH_PULL, VET6_GPIO_NOPULL);

        /* 默认关闭LED */
        VET6_LED1_OFF();
        VET6_LED2_OFF();
    }

    /**
     * @brief 初始化按键引脚
     */
    static inline void vet6_bsp_init_keys(void)
    {
        vet6_gpio_config_input(VET6_KEY1_PIN, VET6_GPIO_PULL_UP);
    }

#ifdef __cplusplus
}
#endif

#endif /* __VET6_BSP_H */
