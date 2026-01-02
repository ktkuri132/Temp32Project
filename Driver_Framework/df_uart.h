#ifndef __DF_UART_H
#define __DF_UART_H

#include <stdbool.h>
#include <stdint.h>
#include <dev_frame.h>

/**
 * @brief UART 设备类型定义
 * @note 所有接口统一使用 int (*)(df_arg_t) 类型
 */
typedef struct df_uart_struct
{
  bool init_flag;                         // UART初始化标志
  int num;                                // UART编号
  uint32_t baudrate;                      // 波特率
  char *name;                             // UART名称
  int (*init)(df_arg_t);                  // 初始化UART，传参arg_null
  int (*deinit)(df_arg_t);                // 关闭UART，传参arg_null
  int (*send)(df_arg_t);                  // 发送数据，传参arg_ptr(data)
  int (*printf)(const char *format, ...); // 格式化输出函数（特殊接口）
  int (*receive)(df_arg_t);               // 接收数据，传参arg_ptr(buffer)
  int (*send_dma)(df_arg_t);              // DMA发送，传参arg_ptr(data)
  int (*receive_dma)(df_arg_t);           // DMA接收，传参arg_ptr(buffer)
} df_uart_t;

#endif /* __DF_UART_H */