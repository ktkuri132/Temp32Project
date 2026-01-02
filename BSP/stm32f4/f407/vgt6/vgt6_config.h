/**
 * @file vgt6_config.h
 * @brief STM32F407VGT6 配置文件
 * @details VGT6特定的配置定义
 */

#ifndef __VGT6_CONFIG_H
#define __VGT6_CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif

/*===========================================================================*/
/*                              芯片配置                                      */
/*===========================================================================*/

/* Flash大小 - VGT6为1MB */
#define VGT6_FLASH_SIZE_KB 1024

/* 与VET6的区别 */
#define VGT6_IS_1M_FLASH 1

/*===========================================================================*/
/*                              外设启用配置                                  */
/*===========================================================================*/

/* 默认启用所有外设 - 可根据需要注释禁用 */
#define VGT6_USE_GPIO 1
#define VGT6_USE_SPI 1
#define VGT6_USE_I2C 1
#define VGT6_USE_USART 1
#define VGT6_USE_TIM 1
#define VGT6_USE_ADC 1
#define VGT6_USE_DAC 1

#ifdef __cplusplus
}
#endif

#endif /* __VGT6_CONFIG_H */
