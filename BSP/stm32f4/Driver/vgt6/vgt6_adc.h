/**
 * @file vgt6_adc.h
 * @brief STM32F407VGT6 ADC驱动 - 复用VET6驱动
 */

#ifndef __VGT6_ADC_H
#define __VGT6_ADC_H

#include "../vet6/vet6_adc.h"

/* 类型别名 */
typedef vet6_adc_t vgt6_adc_t;
typedef vet6_adc_channel_t vgt6_adc_channel_t;
typedef vet6_adc_resolution_t vgt6_adc_resolution_t;
typedef vet6_adc_sampletime_t vgt6_adc_sampletime_t;
typedef vet6_adc_mode_t vgt6_adc_mode_t;
typedef vet6_adc_config_t vgt6_adc_config_t;
typedef vet6_adc_handle_t vgt6_adc_handle_t;

/* 枚举别名 */
#define VGT6_ADC1 VET6_ADC1
#define VGT6_ADC2 VET6_ADC2
#define VGT6_ADC3 VET6_ADC3
#define VGT6_ADC_MAX VET6_ADC_MAX

#define VGT6_ADC_CH0 VET6_ADC_CH0
#define VGT6_ADC_CH1 VET6_ADC_CH1
#define VGT6_ADC_CH2 VET6_ADC_CH2
#define VGT6_ADC_CH3 VET6_ADC_CH3
#define VGT6_ADC_CH4 VET6_ADC_CH4
#define VGT6_ADC_CH5 VET6_ADC_CH5
#define VGT6_ADC_CH6 VET6_ADC_CH6
#define VGT6_ADC_CH7 VET6_ADC_CH7
#define VGT6_ADC_CH8 VET6_ADC_CH8
#define VGT6_ADC_CH9 VET6_ADC_CH9
#define VGT6_ADC_CH10 VET6_ADC_CH10
#define VGT6_ADC_CH11 VET6_ADC_CH11
#define VGT6_ADC_CH12 VET6_ADC_CH12
#define VGT6_ADC_CH13 VET6_ADC_CH13
#define VGT6_ADC_CH14 VET6_ADC_CH14
#define VGT6_ADC_CH15 VET6_ADC_CH15
#define VGT6_ADC_CH_TEMP VET6_ADC_CH_TEMP
#define VGT6_ADC_CH_VREFINT VET6_ADC_CH_VREFINT
#define VGT6_ADC_CH_VBAT VET6_ADC_CH_VBAT

#define VGT6_ADC_RES_12BIT VET6_ADC_RES_12BIT
#define VGT6_ADC_RES_10BIT VET6_ADC_RES_10BIT
#define VGT6_ADC_RES_8BIT VET6_ADC_RES_8BIT
#define VGT6_ADC_RES_6BIT VET6_ADC_RES_6BIT

#define VGT6_ADC_MODE_SINGLE VET6_ADC_MODE_SINGLE
#define VGT6_ADC_MODE_CONTINUOUS VET6_ADC_MODE_CONTINUOUS
#define VGT6_ADC_MODE_SCAN VET6_ADC_MODE_SCAN

/* 函数别名 */
#define vgt6_adc_clk_enable vet6_adc_clk_enable
#define vgt6_adc_clk_disable vet6_adc_clk_disable
#define vgt6_adc_init vet6_adc_init
#define vgt6_adc_init_simple vet6_adc_init_simple
#define vgt6_adc_deinit vet6_adc_deinit
#define vgt6_adc_enable vet6_adc_enable
#define vgt6_adc_disable vet6_adc_disable
#define vgt6_adc_config_channel vet6_adc_config_channel
#define vgt6_adc_start vet6_adc_start
#define vgt6_adc_stop vet6_adc_stop
#define vgt6_adc_wait_eoc vet6_adc_wait_eoc
#define vgt6_adc_read vet6_adc_read
#define vgt6_adc_read_channel vet6_adc_read_channel
#define vgt6_adc_read_average vet6_adc_read_average
#define vgt6_adc_read_temperature vet6_adc_read_temperature
#define vgt6_adc_read_vrefint vet6_adc_read_vrefint
#define vgt6_adc_read_vbat vet6_adc_read_vbat
#define vgt6_adc_to_voltage vet6_adc_to_voltage

#endif /* __VGT6_ADC_H */
