#ifndef DF_DEV_FRAME_H
#define DF_DEV_FRAME_H

#include <df_log.h> // 终端样式和日志
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

// ============ 错误码定义 ============
typedef enum
{
  DF_OK = 0,              // 操作成功
  DF_ERR_PARAM = -1,      // 参数错误
  DF_ERR_NOT_FOUND = -2,  // 设备未找到
  DF_ERR_TIMEOUT = -3,    // 超时
  DF_ERR_BUSY = -4,       // 设备忙
  DF_ERR_NO_MEM = -5,     // 内存不足
  DF_ERR_HW_FAIL = -6,    // 硬件故障
  DF_ERR_NOT_INIT = -7,   // 未初始化
  DF_ERR_ALREADY = -8,    // 已经执行过
  DF_ERR_NOT_SUPPORT = -9 // 不支持的操作
} df_err_t;

// ============ 设备状态定义 ============
typedef enum
{
  DF_STATE_UNINITIALIZED = 0, // 未初始化
  DF_STATE_INITIALIZED = 1,   // 已初始化
  DF_STATE_ENABLED = 2,       // 已启用
  DF_STATE_DISABLED = 3,      // 已禁用
  DF_STATE_ERROR = -1,        // 错误状态
  DF_STATE_BUSY = 4           // 忙碌状态
} df_state_t;

// ============ 参数传递联合体 ============
typedef union
{
  int32_t s32;
  uint32_t us32;
  void *ptr;
  void **argv;
} df_arg_t;

// 默认不传参
#define arg_null ((df_arg_t){.s32 = 0, .us32 = 0, .ptr = NULL, .argv = NULL})
// 传递有符号整型
#define arg_s32(v) ((df_arg_t){.s32 = v})
// 传递无符号整型
#define arg_u32(v) ((df_arg_t){.us32 = v})
// 传递指针
#define arg_ptr(v) ((df_arg_t){.ptr = v})
// 传递参数列表
#define arg_argv(v) ((df_arg_t){.argv = v})
// 辅助宏定义
#define ptr(v) ((void *)(v))
#define argv(...) (void*[]){__VA_ARGS__}

#define DF_DEV_END \
  {.name = "", .init = NULL, .enable = NULL, .disable = NULL}

// ============ 控制命令定义 ============
typedef enum
{
  DF_CTRL_GET_STATUS = 0x01, // 获取状态
  DF_CTRL_RESET = 0x02,      // 复位设备
  DF_CTRL_SET_POWER = 0x03,  // 设置电源
  DF_CTRL_GET_INFO = 0x04,   // 获取设备信息
  DF_CTRL_CUSTOM = 0x80      // 自定义命令起始
} df_ctrl_cmd_t;

// ============ 设备模型核心结构 ============
typedef struct df_dev_struct
{
  int index;         // 设备索引
  char name[20];     // 设备名称
  df_state_t status; // 设备状态（使用枚举）
  df_arg_t arg;      // 设备参数
  uint8_t ref_count; // 引用计数

  // 基础操作接口
  int (*init)(df_arg_t);           // 初始化函数指针
  int (*deinit)(df_arg_t);         // 反初始化函数指针
  int (*open)(df_arg_t);           // 打开设备
  int (*close)(df_arg_t);          // 关闭设备
  int (*read)(df_arg_t);           // 读取函数指针
  int (*write)(df_arg_t);          // 写入函数指针
  int (*enable)(df_arg_t);         // 启动函数指针
  int (*disable)(df_arg_t);        // 停止函数指针
  int (*ioctl)(int cmd, df_arg_t); // 控制命令接口

  void **priv;   // 私有数据指针
} df_dev_t;

// ============ 核心函数声明 ============
int df_dev_register(df_dev_t dev_info[]);
int df_dev_find(df_dev_t dev_info[], const char *name, df_dev_t *device);
int df_dev_open(df_dev_t *device);
int df_dev_close(df_dev_t *device);
int df_dev_enable(df_dev_t *device);
int df_dev_disable(df_dev_t *device);
int df_dev_ioctl(df_dev_t *device, int cmd, df_arg_t arg);
const char *df_err_to_str(df_err_t err);

#endif