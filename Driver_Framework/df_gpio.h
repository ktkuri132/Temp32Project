#ifndef __DF_GPIO_H
#define __DF_GPIO_H

#include <stdbool.h>
#include <stdint.h>
#include <dev_frame.h>

/**
 * @brief GPIO 设备类型定义
 * @note 所有接口统一使用 int (*)(df_arg_t) 类型
 */
typedef struct df_gpio_struct
{
  bool init_flag;            // GPIO初始化标志
  bool state;                // GPIO状态（0: 低电平, 1: 高电平）
  int group;                 // GPIO组号
  int pin;                   // GPIO引脚号
  char *name;                // GPIO名称
  int (*init)(df_arg_t);     // 初始化GPIO，传参arg_null
  int (*deinit)(df_arg_t);   // 关闭GPIO，传参arg_null
  int (*set_high)(df_arg_t); // 设置GPIO高电平，传参arg_null
  int (*set_low)(df_arg_t);  // 设置GPIO低电平，传参arg_null
  int (*toggle)(df_arg_t);   // 翻转GPIO状态，传参arg_null
  int (*read)(df_arg_t);     // 读取GPIO状态，传递arg_u32(value)
} df_gpio_t;

#endif /* __DF_GPIO_H */
