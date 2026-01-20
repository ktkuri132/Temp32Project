#ifndef __DF_LED_H
#define __DF_LED_H

#include <stdbool.h>
#include <stdint.h>
#include <dev_frame.h>

/**
 * @brief LED 设备类型定义
 * @note 所有接口统一使用 int (*)(df_arg_t) 类型
 */
typedef struct df_led_struct
{
  bool init_flag;          // LED初始化标志
  uint8_t num;             // LED编号
  bool state;              // LED状态（0: 关闭, 1: 打开）
  char *name;              // LED名称
  int (*init)(df_arg_t);   // 初始化LED，传参arg_null
  int (*on)(df_arg_t);     // 打开LED，传参arg_null
  int (*off)(df_arg_t);    // 关闭LED，传参arg_null
  int (*toggle)(df_arg_t); // 翻转LED，传参arg_null
} df_led_t;

/**
 * @brief LED 事件运行时类型定义
 * @note led_read 接口统一为 int (*)(df_arg_t)，传参 arg_ptr(df_led_t*)
 */
typedef struct df_led_event_struct
{
  uint32_t sys_runtime;      // 系统运行时间
  uint32_t blink_interval;   // 闪烁时间间隔（毫秒）
  int (*led_read)(df_arg_t); // LED读取函数，传参 arg_ptr(df_led_t*)
} df_led_event_t;

#endif