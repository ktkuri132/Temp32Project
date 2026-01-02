/**
 * @file vgt6_bsp.h
 * @brief STM32F407VGT6 板级支持包统一头文件
 * @details 包含所有VGT6板级驱动的头文件，便于用户使用
 *
 * @note VGT6与VET6的区别:
 *       - Flash: 1MB (VET6为512KB)
 *       - 其他外设资源相同
 *
 * @section MCU特性
 * - MCU: STM32F407VGT6
 * - Core: ARM Cortex-M4
 * - Flash: 1MB
 * - SRAM: 192KB (128KB + 64KB CCM)
 * - 封装: LQFP100
 * - GPIO: 82个可用IO (PA0-PA15, PB0-PB15, PC0-PC15, PD0-PD15, PE0-PE15)
 *
 * @section 外设资源 (与VET6相同)
 * - GPIO: 5个端口 (GPIOA-GPIOE)
 * - TIM: 14个定时器 (TIM1-TIM14)
 * - SPI: 3个 (SPI1-SPI3)
 * - I2C: 3个 (I2C1-I2C3)
 * - USART: 6个 (USART1-USART3, UART4-UART5, USART6)
 * - ADC: 3个 (ADC1-ADC3), 16个外部通道+3个内部通道
 * - DAC: 2通道
 */

#ifndef __VGT6_BSP_H
#define __VGT6_BSP_H

#ifdef __cplusplus
extern "C"
{
#endif

/*===========================================================================*/
/*                              VGT6与VET6驱动共享                             */
/*===========================================================================*/

/*
 * VGT6和VET6的外设配置完全相同，仅Flash大小不同
 * 因此VGT6直接复用VET6的驱动代码
 */

/* GPIO驱动 */
#include "vgt6_gpio.h"

/* SPI驱动 */
#include "vgt6_spi.h"

/* I2C驱动 */
#include "vgt6_i2c.h"

/* USART驱动 */
#include "vgt6_usart.h"

/* 定时器驱动 */
#include "vgt6_tim.h"

/* ADC驱动 */
#include "vgt6_adc.h"

/* DAC驱动 */
#include "vgt6_dac.h"

    /*===========================================================================*/
    /*                              版本信息                                      */
    /*===========================================================================*/

#define VGT6_BSP_VERSION_MAJOR 1
#define VGT6_BSP_VERSION_MINOR 0
#define VGT6_BSP_VERSION_PATCH 0
#define VGT6_BSP_VERSION ((VGT6_BSP_VERSION_MAJOR << 16) | \
                          (VGT6_BSP_VERSION_MINOR << 8) |  \
                          VGT6_BSP_VERSION_PATCH)
#define VGT6_BSP_VERSION_STRING "1.0.0"

    /*===========================================================================*/
    /*                              芯片信息                                      */
    /*===========================================================================*/

#define VGT6_MCU_NAME "STM32F407VGT6"
#define VGT6_CORE "ARM Cortex-M4"
#define VGT6_FLASH_SIZE (1024 * 1024) /* 1MB */
#define VGT6_SRAM_SIZE (128 * 1024)   /* 128KB Main SRAM */
#define VGT6_CCMRAM_SIZE (64 * 1024)  /* 64KB CCM RAM */
#define VGT6_PACKAGE "LQFP100"
#define VGT6_GPIO_PORTS 5       /* GPIOA-GPIOE */
#define VGT6_GPIO_PINS_TOTAL 80 /* 5 * 16 */

    /*===========================================================================*/
    /*                              时钟信息                                      */
    /*===========================================================================*/

#define VGT6_SYSCLK_FREQ 168000000UL   /* 168 MHz */
#define VGT6_HCLK_FREQ 168000000UL     /* AHB = 168 MHz */
#define VGT6_APB1_FREQ 42000000UL      /* APB1 = 42 MHz */
#define VGT6_APB2_FREQ 84000000UL      /* APB2 = 84 MHz */
#define VGT6_APB1_TIM_FREQ 84000000UL  /* APB1 Timer = 84 MHz */
#define VGT6_APB2_TIM_FREQ 168000000UL /* APB2 Timer = 168 MHz */

/*===========================================================================*/
/*                              典型应用引脚定义                               */
/*===========================================================================*/

/**
 * @brief LED引脚定义 (根据实际硬件修改)
 */
#ifndef VGT6_LED1_PIN
#define VGT6_LED1_PIN VGT6_PE0
#endif

#ifndef VGT6_LED2_PIN
#define VGT6_LED2_PIN VGT6_PE1
#endif

/**
 * @brief 按键引脚定义 (根据实际硬件修改)
 */
#ifndef VGT6_KEY1_PIN
#define VGT6_KEY1_PIN VGT6_PA0
#endif

/**
 * @brief 调试串口定义
 */
#ifndef VGT6_DEBUG_USART
#define VGT6_DEBUG_USART VGT6_USART1
#endif

/*===========================================================================*/
/*                              便捷宏定义                                     */
/*===========================================================================*/

/**
 * @brief LED控制宏
 */
#define VGT6_LED1_ON() vgt6_gpio_reset(VGT6_LED1_PIN)
#define VGT6_LED1_OFF() vgt6_gpio_set(VGT6_LED1_PIN)
#define VGT6_LED1_TOGGLE() vgt6_gpio_toggle(VGT6_LED1_PIN)

#define VGT6_LED2_ON() vgt6_gpio_reset(VGT6_LED2_PIN)
#define VGT6_LED2_OFF() vgt6_gpio_set(VGT6_LED2_PIN)
#define VGT6_LED2_TOGGLE() vgt6_gpio_toggle(VGT6_LED2_PIN)

/**
 * @brief 按键读取宏
 */
#define VGT6_KEY1_READ() vgt6_gpio_read(VGT6_KEY1_PIN)

    /*===========================================================================*/
    /*                              初始化函数                                     */
    /*===========================================================================*/

    /**
     * @brief 初始化所有GPIO时钟
     */
    static inline void vgt6_bsp_init_gpio_clocks(void)
    {
        vgt6_gpio_clk_enable(VGT6_GPIOA);
        vgt6_gpio_clk_enable(VGT6_GPIOB);
        vgt6_gpio_clk_enable(VGT6_GPIOC);
        vgt6_gpio_clk_enable(VGT6_GPIOD);
        vgt6_gpio_clk_enable(VGT6_GPIOE);
    }

    /**
     * @brief 初始化LED引脚
     * @note 假设LED低电平点亮
     */
    static inline void vgt6_bsp_init_leds(void)
    {
        vgt6_gpio_config_output(VGT6_LED1_PIN, VGT6_GPIO_SPEED_LOW, VGT6_GPIO_PUSH_PULL, VGT6_GPIO_NOPULL);
        vgt6_gpio_config_output(VGT6_LED2_PIN, VGT6_GPIO_SPEED_LOW, VGT6_GPIO_PUSH_PULL, VGT6_GPIO_NOPULL);

        /* 默认关闭LED */
        VGT6_LED1_OFF();
        VGT6_LED2_OFF();
    }

    /**
     * @brief 初始化按键引脚
     */
    static inline void vgt6_bsp_init_keys(void)
    {
        vgt6_gpio_config_input(VGT6_KEY1_PIN, VGT6_GPIO_PULL_UP);
    }

#ifdef __cplusplus
}
#endif

#endif /* __VGT6_BSP_H */
