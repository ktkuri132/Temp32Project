/**
 * @file vgt6_gpio.h
 * @brief STM32F407VGT6 GPIO驱动 - 复用VET6驱动
 * @details VGT6与VET6外设配置相同，通过宏定义复用VET6代码
 */

#ifndef __VGT6_GPIO_H
#define __VGT6_GPIO_H

/* 复用VET6 GPIO驱动 */
#include "../vet6/vet6_gpio.h"

/* 类型别名 */
typedef vet6_gpio_port_t vgt6_gpio_port_t;
typedef vet6_gpio_id_t vgt6_gpio_id_t;
typedef vet6_gpio_mode_t vgt6_gpio_mode_t;
typedef vet6_gpio_speed_t vgt6_gpio_speed_t;
typedef vet6_gpio_otype_t vgt6_gpio_otype_t;
typedef vet6_gpio_pull_t vgt6_gpio_pull_t;
typedef vet6_gpio_af_t vgt6_gpio_af_t;

/* 端口枚举别名 */
#define VGT6_GPIOA VET6_GPIOA
#define VGT6_GPIOB VET6_GPIOB
#define VGT6_GPIOC VET6_GPIOC
#define VGT6_GPIOD VET6_GPIOD
#define VGT6_GPIOE VET6_GPIOE
#define VGT6_GPIO_MAX VET6_GPIO_MAX

/* 引脚ID别名 - Port A */
#define VGT6_PA0 VET6_PA0
#define VGT6_PA1 VET6_PA1
#define VGT6_PA2 VET6_PA2
#define VGT6_PA3 VET6_PA3
#define VGT6_PA4 VET6_PA4
#define VGT6_PA5 VET6_PA5
#define VGT6_PA6 VET6_PA6
#define VGT6_PA7 VET6_PA7
#define VGT6_PA8 VET6_PA8
#define VGT6_PA9 VET6_PA9
#define VGT6_PA10 VET6_PA10
#define VGT6_PA11 VET6_PA11
#define VGT6_PA12 VET6_PA12
#define VGT6_PA13 VET6_PA13
#define VGT6_PA14 VET6_PA14
#define VGT6_PA15 VET6_PA15

/* 引脚ID别名 - Port B */
#define VGT6_PB0 VET6_PB0
#define VGT6_PB1 VET6_PB1
#define VGT6_PB2 VET6_PB2
#define VGT6_PB3 VET6_PB3
#define VGT6_PB4 VET6_PB4
#define VGT6_PB5 VET6_PB5
#define VGT6_PB6 VET6_PB6
#define VGT6_PB7 VET6_PB7
#define VGT6_PB8 VET6_PB8
#define VGT6_PB9 VET6_PB9
#define VGT6_PB10 VET6_PB10
#define VGT6_PB11 VET6_PB11
#define VGT6_PB12 VET6_PB12
#define VGT6_PB13 VET6_PB13
#define VGT6_PB14 VET6_PB14
#define VGT6_PB15 VET6_PB15

/* 引脚ID别名 - Port C */
#define VGT6_PC0 VET6_PC0
#define VGT6_PC1 VET6_PC1
#define VGT6_PC2 VET6_PC2
#define VGT6_PC3 VET6_PC3
#define VGT6_PC4 VET6_PC4
#define VGT6_PC5 VET6_PC5
#define VGT6_PC6 VET6_PC6
#define VGT6_PC7 VET6_PC7
#define VGT6_PC8 VET6_PC8
#define VGT6_PC9 VET6_PC9
#define VGT6_PC10 VET6_PC10
#define VGT6_PC11 VET6_PC11
#define VGT6_PC12 VET6_PC12
#define VGT6_PC13 VET6_PC13
#define VGT6_PC14 VET6_PC14
#define VGT6_PC15 VET6_PC15

/* 引脚ID别名 - Port D */
#define VGT6_PD0 VET6_PD0
#define VGT6_PD1 VET6_PD1
#define VGT6_PD2 VET6_PD2
#define VGT6_PD3 VET6_PD3
#define VGT6_PD4 VET6_PD4
#define VGT6_PD5 VET6_PD5
#define VGT6_PD6 VET6_PD6
#define VGT6_PD7 VET6_PD7
#define VGT6_PD8 VET6_PD8
#define VGT6_PD9 VET6_PD9
#define VGT6_PD10 VET6_PD10
#define VGT6_PD11 VET6_PD11
#define VGT6_PD12 VET6_PD12
#define VGT6_PD13 VET6_PD13
#define VGT6_PD14 VET6_PD14
#define VGT6_PD15 VET6_PD15

/* 引脚ID别名 - Port E */
#define VGT6_PE0 VET6_PE0
#define VGT6_PE1 VET6_PE1
#define VGT6_PE2 VET6_PE2
#define VGT6_PE3 VET6_PE3
#define VGT6_PE4 VET6_PE4
#define VGT6_PE5 VET6_PE5
#define VGT6_PE6 VET6_PE6
#define VGT6_PE7 VET6_PE7
#define VGT6_PE8 VET6_PE8
#define VGT6_PE9 VET6_PE9
#define VGT6_PE10 VET6_PE10
#define VGT6_PE11 VET6_PE11
#define VGT6_PE12 VET6_PE12
#define VGT6_PE13 VET6_PE13
#define VGT6_PE14 VET6_PE14
#define VGT6_PE15 VET6_PE15

/* 模式别名 */
#define VGT6_GPIO_MODE_INPUT VET6_GPIO_MODE_INPUT
#define VGT6_GPIO_MODE_OUTPUT VET6_GPIO_MODE_OUTPUT
#define VGT6_GPIO_MODE_AF VET6_GPIO_MODE_AF
#define VGT6_GPIO_MODE_ANALOG VET6_GPIO_MODE_ANALOG

/* 速度别名 */
#define VGT6_GPIO_SPEED_LOW VET6_GPIO_SPEED_LOW
#define VGT6_GPIO_SPEED_MEDIUM VET6_GPIO_SPEED_MEDIUM
#define VGT6_GPIO_SPEED_FAST VET6_GPIO_SPEED_FAST
#define VGT6_GPIO_SPEED_HIGH VET6_GPIO_SPEED_HIGH

/* 输出类型别名 */
#define VGT6_GPIO_PUSH_PULL VET6_GPIO_PUSH_PULL
#define VGT6_GPIO_OPEN_DRAIN VET6_GPIO_OPEN_DRAIN

/* 上下拉别名 */
#define VGT6_GPIO_NOPULL VET6_GPIO_NOPULL
#define VGT6_GPIO_PULL_UP VET6_GPIO_PULL_UP
#define VGT6_GPIO_PULL_DOWN VET6_GPIO_PULL_DOWN

/* 函数别名 */
#define vgt6_gpio_clk_enable vet6_gpio_clk_enable
#define vgt6_gpio_clk_disable vet6_gpio_clk_disable
#define vgt6_gpio_config vet6_gpio_config
#define vgt6_gpio_config_output vet6_gpio_config_output
#define vgt6_gpio_config_input vet6_gpio_config_input
#define vgt6_gpio_config_af vet6_gpio_config_af
#define vgt6_gpio_config_analog vet6_gpio_config_analog
#define vgt6_gpio_set vet6_gpio_set
#define vgt6_gpio_reset vet6_gpio_reset
#define vgt6_gpio_toggle vet6_gpio_toggle
#define vgt6_gpio_read vet6_gpio_read
#define vgt6_gpio_write vet6_gpio_write

#endif /* __VGT6_GPIO_H */
