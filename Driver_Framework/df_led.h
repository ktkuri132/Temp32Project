#ifndef __DF_LED_H
#define __DF_LED_H

#include <stdbool.h>
#include <stdint.h>
#include <dev_frame.h>

typedef struct LED_TypeDef
{
  bool LED_Init_Flag;       // LED初始化标志
  uint8_t LED_Num;          // LED编号
  bool LED_State;           // LED状态（0: 关闭, 1: 打开）
  char *LED_Name;           // LED名称
  int (*init)(dev_arg_t);   // 初始化LED，传参arg_null
  int (*on)(dev_arg_t);     // 打开LED，传参arg_null
  int (*off)(dev_arg_t);    // 关闭LED，传参arg_null
  int (*toggle)(dev_arg_t); // 翻转LED，传参arg_null
} Lt;

typedef struct LED_Event_RunTime_TypeDef
{
  uint32_t Sys_RunTime;    // 系统运行时间
  uint32_t LED_Blink_Time; // 闪烁时间间隔（毫秒）
  void (*LED_Read)(Lt *);  // LED读取函数指针
} Lert;

#endif