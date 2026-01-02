/**
 * @file f407_bsp.h
 * @brief STM32F407 系列板级支持包 (BSP) 主头文件
 *
 * 包含所有 F407 外设驱动的头文件，提供统一的接口
 * 此驱动适用于所有 STM32F407 系列芯片，包括但不限于：
 *   - STM32F407VET6 (512KB Flash, LQFP100)
 *   - STM32F407VGT6 (1MB Flash, LQFP100)
 *   - STM32F407ZET6 (512KB Flash, LQFP144)
 *   - STM32F407ZGT6 (1MB Flash, LQFP144)
 *   - STM32F407IGT6 (1MB Flash, LQFP176)
 *
 * @note 不同封装的芯片主要区别：
 *       - Flash 大小不同 (VET6/ZET6 = 512KB, VGT6/ZGT6/IGT6 = 1MB)
 *       - 可用 GPIO 数量不同
 *       - 其他外设功能完全相同
 */

#ifndef __F407_BSP_H
#define __F407_BSP_H

#include <stm32f4xx.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /*===========================================================================*/
    /*                          驱动头文件包含                                    */
    /*===========================================================================*/

#include "f407_gpio.h"  /* GPIO 驱动 */
#include "f407_usart.h" /* USART 驱动 */
#include "f407_spi.h"   /* SPI 驱动 */
#include "f407_i2c.h"   /* I2C 驱动 */
#include "f407_tim.h"   /* 定时器驱动 */
#include "f407_adc.h"   /* ADC 驱动 */
#include "f407_dac.h"   /* DAC 驱动 */

    /*===========================================================================*/
    /*                          芯片信息定义                                      */
    /*===========================================================================*/

/**
 * @brief 芯片系列标识
 */
#define F407_CHIP_SERIES "STM32F407"

/**
 * @brief 系统时钟配置
 * @note 假设使用 8MHz 外部晶振，PLL 配置为 168MHz 主频
 */
#define F407_SYSCLK_FREQ 168000000UL /* 系统时钟 168MHz */
#define F407_HCLK_FREQ 168000000UL   /* AHB 时钟 168MHz */
#define F407_PCLK1_FREQ 42000000UL   /* APB1 时钟 42MHz */
#define F407_PCLK2_FREQ 84000000UL   /* APB2 时钟 84MHz */

    /*===========================================================================*/
    /*                          BSP 版本信息                                      */
    /*===========================================================================*/

#define F407_BSP_VERSION_MAJOR 1
#define F407_BSP_VERSION_MINOR 0
#define F407_BSP_VERSION_PATCH 0

    /**
     * @brief 获取 BSP 版本号
     * @return 版本号 (major << 16 | minor << 8 | patch)
     */
    static inline uint32_t f407_bsp_get_version(void)
    {
        return (F407_BSP_VERSION_MAJOR << 16) |
               (F407_BSP_VERSION_MINOR << 8) |
               F407_BSP_VERSION_PATCH;
    }

#ifdef __cplusplus
}
#endif

#endif /* __F407_BSP_H */
