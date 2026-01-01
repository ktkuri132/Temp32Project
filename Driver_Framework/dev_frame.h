#ifndef DEV_MODEL_CORE_H
#define DEV_MODEL_CORE_H

#include <shell/shell_style.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

// ============ 错误码定义 ============
typedef enum
{
  DEV_OK = 0,              // 操作成功
  DEV_ERR_PARAM = -1,      // 参数错误
  DEV_ERR_NOT_FOUND = -2,  // 设备未找到
  DEV_ERR_TIMEOUT = -3,    // 超时
  DEV_ERR_BUSY = -4,       // 设备忙
  DEV_ERR_NO_MEM = -5,     // 内存不足
  DEV_ERR_HW_FAIL = -6,    // 硬件故障
  DEV_ERR_NOT_INIT = -7,   // 未初始化
  DEV_ERR_ALREADY = -8,    // 已经执行过
  DEV_ERR_NOT_SUPPORT = -9 // 不支持的操作
} dev_err_t;

// ============ 设备状态定义 ============
typedef enum
{
  DEV_STATE_UNINITIALIZED = 0, // 未初始化
  DEV_STATE_INITIALIZED = 1,   // 已初始化
  DEV_STATE_ENABLED = 2,       // 已启用
  DEV_STATE_DISABLED = 3,      // 已禁用
  DEV_STATE_ERROR = -1,        // 错误状态
  DEV_STATE_BUSY = 4           // 忙碌状态
} dev_state_t;

// ============ 参数传递联合体 ============
typedef union
{
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

#define DEV_INFO_END \
  {.name = "", .init = NULL, .enable = NULL, .disable = NULL}

// ============ 控制命令定义 ============
typedef enum
{
  DEV_CTRL_GET_STATUS = 0x01, // 获取状态
  DEV_CTRL_RESET = 0x02,      // 复位设备
  DEV_CTRL_SET_POWER = 0x03,  // 设置电源
  DEV_CTRL_GET_INFO = 0x04,   // 获取设备信息
  DEV_CTRL_CUSTOM = 0x80      // 自定义命令起始
} dev_ctrl_cmd_t;

// ============ 设备模型核心结构 ============
typedef struct Dev_Model_Core_TypeDef
{
  int index;          // 设备索引
  char name[20];      // 设备名称
  dev_state_t status; // 设备状态（使用枚举）
  dev_arg_t arg;      // 设备参数
  uint8_t ref_count;  // 引用计数
  void *priv_data;    // 私有数据指针

  // 基础操作接口
  int (*init)(dev_arg_t);           // 初始化函数指针
  int (*deinit)(dev_arg_t);         // 反初始化函数指针
  int (*open)(dev_arg_t);           // 打开设备
  int (*close)(dev_arg_t);          // 关闭设备
  int (*enable)(dev_arg_t);         // 启动函数指针
  int (*disable)(dev_arg_t);        // 停止函数指针
  int (*ioctl)(int cmd, dev_arg_t); // 控制命令接口
} dev_info_t;

// ============ 核心函数声明 ============
int Device_Registration(dev_info_t dev_info[]);
int Find_Device(dev_info_t dev_info[], const char *name, dev_info_t **device);
int Device_Open(dev_info_t *device);
int Device_Close(dev_info_t *device);
int Device_Enable(dev_info_t *device);
int Device_Disable(dev_info_t *device);
int Device_Ioctl(dev_info_t *device, int cmd, dev_arg_t arg);
const char *Device_Get_Error_String(dev_err_t err);

#endif