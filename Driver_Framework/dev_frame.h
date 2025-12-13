#ifndef DEV_MODEL_CORE_H
#define DEV_MODEL_CORE_H

#include <shell_style.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>


typedef union {
  int32_t s32;
  uint32_t us32;
  void *ptr;
  void **argv;
} dev_arg_t;

// 默认不传参
#define arg_null ((dev_arg_t){.s32 = 0, .us32 = 0, .ptr = NULL, .argv = NULL})
// 传递有符号整型
#define arg_s32(v) ((dev_arg_t){.s32 = v})
// 传递无符号整型
#define arg_u32(v) ((dev_arg_t){.us32 = v})
// 传递void*指针
#define arg_ptr(p) ((dev_arg_t){.ptr = p})
// 传递void**指针
#define arg_argv(a) ((dev_arg_t){.argv = a})

// 设备模型核心头文件
typedef struct Dev_Model_Core_TypeDef {
  int index;                 // 设备索引
  char name[20];      // 设备名称
  int status;                // 设备状态
  dev_arg_t arg;             // 设备参数
  int (*init)(dev_arg_t);    // 初始化函数指针
  int (*enable)(dev_arg_t);  // 启动函数指针
  int (*disable)(dev_arg_t); // 停止函数指针
} dev_info_t;

int Device_Registration(dev_info_t dev_info[]);

#endif