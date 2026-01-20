/**
 * @file f103_bsp.h
 * @brief STM32F103 系列 BSP 统一头文件
 * @note 包含所有 F103 外设驱动头文件
 */

#ifndef __F103_BSP_H
#define __F103_BSP_H

#ifdef __cplusplus
extern "C"
{
#endif

/* F103 系列标识 */
#define F103_SERIES 1

/* CMSIS 头文件 */
#include "stm32f10x.h"

/* F103 系列驱动头文件 */
#include "f103_gpio.h"
#include "f103_usart.h"
#include "f103_spi.h"
#include "f103_i2c.h"
#include "f103_tim.h"
#include "f103_adc.h"

#ifdef __cplusplus
}
#endif

#endif /* __F103_BSP_H */
