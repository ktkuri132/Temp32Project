/**
 * @file vet6_config.h
 * @brief STM32F407VET6 配置文件
 * @details VET6特定的配置定义
 */

#ifndef __VET6_CONFIG_H
#define __VET6_CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif

/*===========================================================================*/
/*                              芯片配置                                      */
/*===========================================================================*/

/* Flash大小 - VET6为512KB */
#define VET6_FLASH_SIZE_KB 512

/* 与VGT6的区别 */
#define VET6_IS_512K_FLASH 1

/*===========================================================================*/
/*                              外设启用配置                                  */
/*===========================================================================*/

/* 默认启用所有外设 - 可根据需要注释禁用 */
#define VET6_USE_GPIO 1
#define VET6_USE_SPI 1
#define VET6_USE_I2C 1
#define VET6_USE_USART 1
#define VET6_USE_TIM 1
#define VET6_USE_ADC 1
#define VET6_USE_DAC 1

#ifdef __cplusplus
}
#endif

#endif /* __VET6_CONFIG_H */
