/**
 * @file vgt6_bsp.h
 * @brief STM32F407VGT6 板级支持包统一头文件
 * @details 包含所有VGT6板级驱动的头文件，便于用户使用
 *          VGT6与F407系列共享驱动代码，仅通过宏名称区分
 *
 * @note VGT6与VET6的区别:
 *       - Flash: 1MB (VET6为512KB)
 *       - 其他外设资源完全相同
 *
 * @section MCU特性
 * - MCU: STM32F407VGT6
 * - Core: ARM Cortex-M4
 * - Flash: 1MB
 * - SRAM: 192KB (128KB + 64KB CCM)
 * - 封装: LQFP100
 * - GPIO: 82个可用IO (PA0-PA15, PB0-PB15, PC0-PC15, PD0-PD15, PE0-PE15)
 */

#ifndef __VGT6_BSP_H
#define __VGT6_BSP_H

#ifdef __cplusplus
extern "C"
{
#endif

    /*===========================================================================*/
    /*                              版本信息                                      */
    /*===========================================================================*/

#define VGT6_BSP_VERSION_MAJOR 1
#define VGT6_BSP_VERSION_MINOR 1
#define VGT6_BSP_VERSION_PATCH 0
#define VGT6_BSP_VERSION ((VGT6_BSP_VERSION_MAJOR << 16) | \
                          (VGT6_BSP_VERSION_MINOR << 8) |  \
                          VGT6_BSP_VERSION_PATCH)
#define VGT6_BSP_VERSION_STRING "1.1.0"

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
    /*                              外设资源                                      */
    /*===========================================================================*/

#define VGT6_TIM_COUNT 14  /* TIM1-TIM14 */
#define VGT6_SPI_COUNT 3   /* SPI1-SPI3 */
#define VGT6_I2C_COUNT 3   /* I2C1-I2C3 */
#define VGT6_USART_COUNT 6 /* USART1-3, UART4-5, USART6 */
#define VGT6_ADC_COUNT 3   /* ADC1-ADC3 */
#define VGT6_DAC_COUNT 2   /* DAC CH1-CH2 */

#ifdef __cplusplus
}
#endif

#endif /* __VGT6_BSP_H */
