#ifndef __DF_ADC_H
#define __DF_ADC_H

#include <stdbool.h>
#include <stdint.h>
#include <dev_frame.h>

/**
 * @brief ADC 设备类型定义
 * @note 所有接口统一使用 int (*)(df_arg_t) 类型
 */
typedef struct df_adc_struct
{
  bool init_flag;             // ADC初始化标志
  uint8_t num;                // ADC编号
  char *name;                 // ADC名称
  int (*init)(df_arg_t);      // 初始化ADC，传参arg_null
  int (*deinit)(df_arg_t);    // 关闭ADC，传参arg_null
  int (*get_value)(df_arg_t); // 获取ADC数值，传参arg_u32(channel)，返回ADC值
} df_adc_t;

#endif