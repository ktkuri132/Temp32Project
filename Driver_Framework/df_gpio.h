#ifndef __DF_GPIO_H
#define __DF_GPIO_H

#include <stdbool.h>
#include <stdint.h>
#include <dev_frame.h>

typedef struct GPIO_TypeDef
{
  bool GPIO_Init_Flag;        // GPIO初始化标志
  bool GPIO_State;            // GPIO状态（0: 低电平, 1: 高电平）
  int GPIO_gourp;             // GPIO组号
  int GPIO_Pin;               // GPIO引脚号
  char *GPIO_Name;            // GPIO名称
  int (*init)(dev_arg_t);     // 初始化GPIO，传参arg_null
  int (*deinit)(dev_arg_t);   // 关闭GPIO，传参arg_null
  int (*set_high)(dev_arg_t); // 设置GPIO高电平，传参arg_null
  int (*set_low)(dev_arg_t);  // 设置GPIO低电平，传参arg_null
  int (*toggle)(dev_arg_t);   // 翻转GPIO状态，传参arg_null
  int (*read)(dev_arg_t);     // 读取GPIO状态，传递arg_u32(value)
} Gt;
