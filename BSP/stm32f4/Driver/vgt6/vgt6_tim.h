/**
 * @file vgt6_tim.h
 * @brief STM32F407VGT6 Timer驱动 - 复用VET6驱动
 */

#ifndef __VGT6_TIM_H
#define __VGT6_TIM_H

#include "../vet6/vet6_tim.h"

/* 类型别名 */
typedef vet6_tim_t vgt6_tim_t;
typedef vet6_tim_type_t vgt6_tim_type_t;
typedef vet6_tim_config_t vgt6_tim_config_t;
typedef vet6_tim_handle_t vgt6_tim_handle_t;

/* 枚举别名 */
#define VGT6_TIM1 VET6_TIM1
#define VGT6_TIM2 VET6_TIM2
#define VGT6_TIM3 VET6_TIM3
#define VGT6_TIM4 VET6_TIM4
#define VGT6_TIM5 VET6_TIM5
#define VGT6_TIM6 VET6_TIM6
#define VGT6_TIM7 VET6_TIM7
#define VGT6_TIM8 VET6_TIM8
#define VGT6_TIM9 VET6_TIM9
#define VGT6_TIM10 VET6_TIM10
#define VGT6_TIM11 VET6_TIM11
#define VGT6_TIM12 VET6_TIM12
#define VGT6_TIM13 VET6_TIM13
#define VGT6_TIM14 VET6_TIM14
#define VGT6_TIM_MAX VET6_TIM_MAX

/* 函数别名 */
#define vgt6_tim_clk_enable vet6_tim_clk_enable
#define vgt6_tim_clk_disable vet6_tim_clk_disable
#define vgt6_tim_init vet6_tim_init
#define vgt6_tim_init_us vet6_tim_init_us
#define vgt6_tim_init_ms vet6_tim_init_ms
#define vgt6_tim_deinit vet6_tim_deinit
#define vgt6_tim_enable vet6_tim_enable
#define vgt6_tim_disable vet6_tim_disable
#define vgt6_tim_get_counter vet6_tim_get_counter
#define vgt6_tim_set_counter vet6_tim_set_counter
#define vgt6_tim_enable_irq vet6_tim_enable_irq
#define vgt6_tim_disable_irq vet6_tim_disable_irq

/* PWM函数别名 */
#define vgt6_pwm_init vet6_pwm_init
#define vgt6_pwm_init_quick vet6_pwm_init_quick
#define vgt6_pwm_set_duty vet6_pwm_set_duty
#define vgt6_pwm_set_freq vet6_pwm_set_freq
#define vgt6_pwm_enable vet6_pwm_enable
#define vgt6_pwm_disable vet6_pwm_disable

/* 编码器函数别名 */
#define vgt6_encoder_init vet6_encoder_init
#define vgt6_encoder_get_count vet6_encoder_get_count
#define vgt6_encoder_reset vet6_encoder_reset

#endif /* __VGT6_TIM_H */
