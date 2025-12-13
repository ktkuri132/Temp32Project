#ifndef __DF_ADC_H
#define __DF_ADC_H

#include <stdbool.h>
#include <stdint.h>
#include <dev_frame.h>
#ifndef DEV_MODEL_CORE_H

typedef union {
  int32_t s32;
  void *ptr;
  void **argv;
} dev_arg_t;

#define arg_null ((dev_arg_t){.s32 = 0, .us32 = 0, .ptr = NULL, .argv = NULL})
#define arg_s32(v) ((dev_arg_t){.s32 = v})
#define arg_u32(v) ((dev_arg_t){.us32 = v})
#define arg_ptr(p) ((dev_arg_t){.ptr = p})
#define arg_argv(a) ((dev_arg_t){.argv = a})

#endif


typedef struct ADC_TypeDef {
    bool ADC_Init_Flag;      // ADC初始化标志
    uint8_t ADC_Num;         // ADC编号
    char *ADC_Name;      // ADC名称
    int (*init)(dev_arg_t);       // 初始化ADC，传参arg_null
    int (*deinit)(dev_arg_t);     // 关闭ADC，传参arg_null
    int (*get_value)(dev_arg_t); // 获取ADC数值，传递arg_u32(value)
} At;

#endif