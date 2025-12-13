#ifndef __DF_UART_H
#define __DF_UART_H

#include <stdbool.h>
#include <stdint.h>
#include <dev_frame.h>
#ifndef DEV_MODEL_CORE_H

typedef union
{
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

typedef struct UART_TypeDef
{
  bool UART_Init_Flag;               // UART初始化标志
  int UART_Num;                      // UART编号
  char *UART_Name;                   // UART名称
  int (*init)(dev_arg_t);            // 初始化UART，传参arg_null
  int (*deinit)(dev_arg_t);          // 关闭UART，传参arg_null
  int (*send)(dev_arg_t);            // 发送数据，传递arg_ptr(data)
  int (*receive)(dev_arg_t);         // 接收数据，传递arg_ptr(buffer)
  int (*send_withDMA)(dev_arg_t);    // DMA方式发送数据，传递arg_ptr(data)
  int (*receive_withDMA)(dev_arg_t); // DMA方式接收数据，传递arg_ptr(buffer)
} Ut;

#endif /* __DF_UART_H */