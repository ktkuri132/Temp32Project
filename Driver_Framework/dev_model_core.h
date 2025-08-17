#ifndef DEV_MODEL_CORE_H
#define DEV_MODEL_CORE_H

#include <shell_style.h>
#include <stddef.h>
#include <stdio.h>

typedef union {
    int32_t s32;
    void *ptr;
    void **argv;
} dev_arg_t;

// 设备模型核心头文件
typedef struct Dev_Model_Core_TypeDef {
    int index;                  // 设备索引
    char device_name[50];       // 设备名称
    int status;                 // 设备状态
    dev_arg_t arg;              // 设备参数
    int (*init)(dev_arg_t);     // 初始化函数指针
    int (*enable)(dev_arg_t);   // 启动函数指针
    int (*disable)(dev_arg_t);  // 停止函数指针
} dev_model_t;

int dev_model_core_init(dev_model_t dev_model[]);

#endif