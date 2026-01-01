/**
 * @file df_log.h
 * @brief 驱动框架统一日志系统
 * @author Driver Framework Team
 * @date 2026-01-01
 */

#ifndef __DF_LOG_H__
#define __DF_LOG_H__

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

// ============ 日志级别定义 ============
typedef enum
{
    LOG_LEVEL_NONE = 0, // 不输出日志
    LOG_LEVEL_ERROR,    // 错误级别
    LOG_LEVEL_WARN,     // 警告级别
    LOG_LEVEL_INFO,     // 信息级别
    LOG_LEVEL_DEBUG,    // 调试级别
    LOG_LEVEL_VERBOSE   // 详细日志
} log_level_t;

// ============ 日志颜色定义（可选）============
#ifdef LOG_USE_COLOR
#define LOG_COLOR_RED "\033[31m"
#define LOG_COLOR_YELLOW "\033[33m"
#define LOG_COLOR_GREEN "\033[32m"
#define LOG_COLOR_CYAN "\033[36m"
#define LOG_COLOR_RESET "\033[0m"
#else
#define LOG_COLOR_RED ""
#define LOG_COLOR_YELLOW ""
#define LOG_COLOR_GREEN ""
#define LOG_COLOR_CYAN ""
#define LOG_COLOR_RESET ""
#endif

// ============ 缓冲模式定义 ============
typedef enum
{
    LOG_BUFFER_MODE_DIRECT = 0,  // 直接输出模式
    LOG_BUFFER_MODE_BUFFERED = 1 // 缓冲模式
} log_buffer_mode_t;

// ============ 缓冲区溢出策略 ============
typedef enum
{
    LOG_OVERFLOW_OVERWRITE = 0, // 覆盖旧数据
    LOG_OVERFLOW_DISCARD = 1    // 丢弃新数据
} log_overflow_policy_t;

// ============ 日志配置 ============
typedef struct
{
    log_level_t level;                     // 当前日志级别
    bool enable_timestamp;                 // 是否启用时间戳
    bool enable_color;                     // 是否启用颜色
    void (*output_func)(const char *);     // 自定义输出函数
    log_buffer_mode_t buffer_mode;         // 缓冲模式
    log_overflow_policy_t overflow_policy; // 溢出策略
} log_config_t;

// ============ 全局日志配置 ============
extern log_config_t g_log_config;

// ============ 日志初始化 ============
void log_init(log_level_t level);
void log_set_level(log_level_t level);
void log_set_output(void (*func)(const char *));
void log_set_timestamp_func(uint32_t (*get_tick)(void)); // 设置时间戳回调函数
void log_enable_timestamp(bool enable);                  // 启用/禁用时间戳
// ============ 缓冲区管理 ============
void log_buffer_init(size_t size);                          // 初始化缓冲区（默认1024字节）
void log_set_buffer_mode(log_buffer_mode_t mode);           // 设置缓冲模式
void log_set_overflow_policy(log_overflow_policy_t policy); // 设置溢出策略
int log_flush(void);                                        // 刷新缓冲区，输出所有日志
void log_buffer_clear(void);                                // 清空缓冲区
size_t log_buffer_get_usage(void);                          // 获取缓冲区使用量
bool log_buffer_is_full(void);                              // 检查缓冲区是否满
// ============ 底层日志函数 ============
void log_print(log_level_t level, const char *tag, const char *fmt, ...);

// ============ 日志宏定义 ============
#define LOG_TAG_DEFAULT "DF"

// 错误日志 - 红色
#define LOG_E(tag, fmt, ...) \
    log_print(LOG_LEVEL_ERROR, tag, LOG_COLOR_RED "[E] " fmt LOG_COLOR_RESET, ##__VA_ARGS__)

// 警告日志 - 黄色
#define LOG_W(tag, fmt, ...) \
    log_print(LOG_LEVEL_WARN, tag, LOG_COLOR_YELLOW "[W] " fmt LOG_COLOR_RESET, ##__VA_ARGS__)

// 信息日志 - 绿色
#define LOG_I(tag, fmt, ...) \
    log_print(LOG_LEVEL_INFO, tag, LOG_COLOR_GREEN "[I] " fmt LOG_COLOR_RESET, ##__VA_ARGS__)

// 调试日志 - 青色
#define LOG_D(tag, fmt, ...) \
    log_print(LOG_LEVEL_DEBUG, tag, LOG_COLOR_CYAN "[D] " fmt LOG_COLOR_RESET, ##__VA_ARGS__)

// 详细日志
#define LOG_V(tag, fmt, ...) \
    log_print(LOG_LEVEL_VERBOSE, tag, "[V] " fmt, ##__VA_ARGS__)

// ============ 带默认标签的简化宏 ============
#define LOGE(fmt, ...) LOG_E(LOG_TAG_DEFAULT, fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) LOG_W(LOG_TAG_DEFAULT, fmt, ##__VA_ARGS__)
#define LOGI(fmt, ...) LOG_I(LOG_TAG_DEFAULT, fmt, ##__VA_ARGS__)
#define LOGD(fmt, ...) LOG_D(LOG_TAG_DEFAULT, fmt, ##__VA_ARGS__)
#define LOGV(fmt, ...) LOG_V(LOG_TAG_DEFAULT, fmt, ##__VA_ARGS__)

// ============ 断言宏 ============
#ifdef LOG_ENABLE_ASSERT
#define LOG_ASSERT(condition, fmt, ...)                                               \
    do                                                                                \
    {                                                                                 \
        if (!(condition))                                                             \
        {                                                                             \
            LOG_E("ASSERT", "Assertion failed: %s, " fmt, #condition, ##__VA_ARGS__); \
            while (1)                                                                 \
                ; /* 进入死循环 */                                                    \
        }                                                                             \
    } while (0)
#else
#define LOG_ASSERT(condition, fmt, ...) ((void)0)
#endif

// ============ 十六进制数据打印 ============
void log_hex_dump(log_level_t level, const char *tag, const void *data, size_t len);

// 便捷宏
#define LOG_HEX_E(tag, data, len) log_hex_dump(LOG_LEVEL_ERROR, tag, data, len)
#define LOG_HEX_I(tag, data, len) log_hex_dump(LOG_LEVEL_INFO, tag, data, len)
#define LOG_HEX_D(tag, data, len) log_hex_dump(LOG_LEVEL_DEBUG, tag, data, len)

#endif /* __DF_LOG_H__ */
