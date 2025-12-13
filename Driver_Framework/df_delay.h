#ifndef __DF_DELAY_H
#define __DF_DELAY_H

#include <stdint.h>
#include <stdbool.h>
#include <dev_frame.h>
#ifndef DEV_MODEL_CORE_H

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

#endif

typedef struct Delay_TypeDef {
    bool Delay_Init_Flag; // 延时模块初始化标志
    int (*init)(dev_arg_t); // 延时模块初始化函数指针
    void (*ms)(uint32_t ms); // 毫秒延时函数
    void (*us)(uint32_t us); // 微秒延时函数
} Dt;

#endif /* __DF_DELAY_H */