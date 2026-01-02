/**
 * @file vgt6_dac.h
 * @brief STM32F407VGT6 DAC驱动 - 复用VET6驱动
 */

#ifndef __VGT6_DAC_H
#define __VGT6_DAC_H

#include "../vet6/vet6_dac.h"

/* 类型别名 */
typedef vet6_dac_channel_t vgt6_dac_channel_t;
typedef vet6_dac_align_t vgt6_dac_align_t;
typedef vet6_dac_trigger_t vgt6_dac_trigger_t;
typedef vet6_dac_wave_t vgt6_dac_wave_t;
typedef vet6_dac_triangle_amp_t vgt6_dac_triangle_amp_t;
typedef vet6_dac_config_t vgt6_dac_config_t;
typedef vet6_dac_handle_t vgt6_dac_handle_t;

/* 枚举别名 */
#define VGT6_DAC_CH1 VET6_DAC_CH1
#define VGT6_DAC_CH2 VET6_DAC_CH2
#define VGT6_DAC_CH_MAX VET6_DAC_CH_MAX

#define VGT6_DAC_ALIGN_12B_R VET6_DAC_ALIGN_12B_R
#define VGT6_DAC_ALIGN_12B_L VET6_DAC_ALIGN_12B_L
#define VGT6_DAC_ALIGN_8B_R VET6_DAC_ALIGN_8B_R

#define VGT6_DAC_WAVE_NONE VET6_DAC_WAVE_NONE
#define VGT6_DAC_WAVE_NOISE VET6_DAC_WAVE_NOISE
#define VGT6_DAC_WAVE_TRIANGLE VET6_DAC_WAVE_TRIANGLE

/* 函数别名 */
#define vgt6_dac_clk_enable vet6_dac_clk_enable
#define vgt6_dac_clk_disable vet6_dac_clk_disable
#define vgt6_dac_init vet6_dac_init
#define vgt6_dac_init_simple vet6_dac_init_simple
#define vgt6_dac_deinit vet6_dac_deinit
#define vgt6_dac_enable vet6_dac_enable
#define vgt6_dac_disable vet6_dac_disable
#define vgt6_dac_set_value vet6_dac_set_value
#define vgt6_dac_set_value_aligned vet6_dac_set_value_aligned
#define vgt6_dac_get_value vet6_dac_get_value
#define vgt6_dac_set_voltage vet6_dac_set_voltage
#define vgt6_dac_software_trigger vet6_dac_software_trigger
#define vgt6_dac_enable_noise vet6_dac_enable_noise
#define vgt6_dac_enable_triangle vet6_dac_enable_triangle
#define vgt6_dac_disable_wave vet6_dac_disable_wave
#define vgt6_dac_enable_dma vet6_dac_enable_dma
#define vgt6_dac_disable_dma vet6_dac_disable_dma
#define vgt6_dac_dual_init vet6_dac_dual_init
#define vgt6_dac_dual_set_value vet6_dac_dual_set_value
#define vgt6_dac_dual_software_trigger vet6_dac_dual_software_trigger

#endif /* __VGT6_DAC_H */
