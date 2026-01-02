#ifndef __DF_DELAY_H
#define __DF_DELAY_H

#include <stdint.h>
#include <stdbool.h>
#include <dev_frame.h>

/**
 * @brief 延时模块类型定义
 * @note 所有接口统一使用 int (*)(df_arg_t) 类型
 *       ms/us 传参使用 arg_u32(delay_value)
 */
typedef struct df_delay_struct
{
  bool init_flag;        // 延时模块初始化标志
  int (*init)(df_arg_t); // 延时模块初始化，传参 arg_null
  int (*ms)(df_arg_t);   // 毫秒延时，传参 arg_u32(ms)
  int (*us)(df_arg_t);   // 微秒延时，传参 arg_u32(us)
} df_delay_t;

/**
 * @brief 便捷延时宏
 * @note 简化调用方式，自动包装参数
 *       使用方式: DF_DELAY_MS(delay, 100); DF_DELAY_US(delay, 50);
 */
#define DF_DELAY_MS(d, val) ((d).ms(arg_u32(val)))
#define DF_DELAY_US(d, val) ((d).us(arg_u32(val)))

#endif /* __DF_DELAY_H */