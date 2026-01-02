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
#include <string.h>

// ============================================================================
//                          终端控制码定义
// ============================================================================

// ------------ 光标控制 ------------
#define CURSOR_UP(n) "\033[" #n "A"                    // 光标上移 n 行
#define CURSOR_DOWN(n) "\033[" #n "B"                  // 光标下移 n 行
#define CURSOR_RIGHT(n) "\033[" #n "C"                 // 光标右移 n 列
#define CURSOR_LEFT(n) "\033[" #n "D"                  // 光标左移 n 列
#define CURSOR_POS(row, col) "\033[" #row ";" #col "H" // 光标移动到 (row, col)
#define CURSOR_HOME "\033[H"                           // 光标移动到左上角 (1,1)
#define CURSOR_SAVE "\033[s"                           // 保存光标位置
#define CURSOR_RESTORE "\033[u"                        // 恢复光标位置
#define CURSOR_HIDE "\033[?25l"                        // 隐藏光标
#define CURSOR_SHOW "\033[?25h"                        // 显示光标
#define CURSOR_GET_POS "\033[6n"                       // 获取光标位置

// ------------ 清屏和清除行 ------------
#define CLEAR_SCREEN "\033[2J"     // 清屏
#define CLEAR_LINE "\033[K"        // 清除从光标到行尾
#define CLEAR_LINE_START "\033[1K" // 清除从行首到光标
#define CLEAR_LINE_ALL "\033[2K"   // 清除整行

// ------------ 文本样式 ------------
#define RESET_ALL "\033[0m"          // 重置所有样式
#define TEXT_BOLD "\033[1m"          // 粗体
#define TEXT_DIM "\033[2m"           // 暗色
#define TEXT_ITALIC "\033[3m"        // 斜体
#define TEXT_UNDERLINE "\033[4m"     // 下划线
#define TEXT_BLINK "\033[5m"         // 闪烁
#define TEXT_REVERSE "\033[7m"       // 反转前景色和背景色
#define TEXT_HIDDEN "\033[8m"        // 隐藏文本
#define TEXT_STRIKETHROUGH "\033[9m" // 删除线

// ------------ 前景色（文字颜色） ------------
#define FG_BLACK "\033[30m"
#define FG_RED "\033[31m"
#define FG_GREEN "\033[32m"
#define FG_YELLOW "\033[33m"
#define FG_BLUE "\033[34m"
#define FG_MAGENTA "\033[35m"
#define FG_CYAN "\033[36m"
#define FG_WHITE "\033[37m"

// ------------ 背景色 ------------
#define BG_BLACK "\033[40m"
#define BG_RED "\033[41m"
#define BG_GREEN "\033[42m"
#define BG_YELLOW "\033[43m"
#define BG_BLUE "\033[44m"
#define BG_MAGENTA "\033[45m"
#define BG_CYAN "\033[46m"
#define BG_WHITE "\033[47m"

// ------------ 256色模式 ------------
#define FG_COLOR_256(n) "\033[38;5;" #n "m" // 前景色，n 为 0-255
#define BG_COLOR_256(n) "\033[48;5;" #n "m" // 背景色，n 为 0-255

// ------------ RGB颜色模式 ------------
#define FG_COLOR_RGB(r, g, b) "\033[38;2;" #r ";" #g ";" #b "m" // 前景色
#define BG_COLOR_RGB(r, g, b) "\033[48;2;" #r ";" #g ";" #b "m" // 背景色

// ------------ 其他控制码 ------------
#define TERM_IDENTIFY "\033[c" // 查询终端类型

// ============================================================================
//                          日志系统定义
// ============================================================================

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
#define LOG_COLOR_RED FG_RED
#define LOG_COLOR_YELLOW FG_YELLOW
#define LOG_COLOR_GREEN FG_GREEN
#define LOG_COLOR_CYAN FG_CYAN
#define LOG_COLOR_MAGENTA FG_MAGENTA
#define LOG_COLOR_BLUE FG_BLUE
#define LOG_COLOR_RESET RESET_ALL
#else
#define LOG_COLOR_RED ""
#define LOG_COLOR_YELLOW ""
#define LOG_COLOR_GREEN ""
#define LOG_COLOR_CYAN ""
#define LOG_COLOR_MAGENTA ""
#define LOG_COLOR_BLUE ""
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
#define LOG_TAG_WIDTH 8 // 标签对齐宽度

// 错误日志 - 红色
#define LOG_E(tag, fmt, ...) \
    log_print(LOG_LEVEL_ERROR, tag, LOG_COLOR_RED "[E]" LOG_COLOR_RESET " %-*s| " fmt, LOG_TAG_WIDTH, tag, ##__VA_ARGS__)

// 警告日志 - 黄色
#define LOG_W(tag, fmt, ...) \
    log_print(LOG_LEVEL_WARN, tag, LOG_COLOR_YELLOW "[W]" LOG_COLOR_RESET " %-*s| " fmt, LOG_TAG_WIDTH, tag, ##__VA_ARGS__)

// 信息日志 - 绿色
#define LOG_I(tag, fmt, ...) \
    log_print(LOG_LEVEL_INFO, tag, LOG_COLOR_GREEN "[I]" LOG_COLOR_RESET " %-*s| " fmt, LOG_TAG_WIDTH, tag, ##__VA_ARGS__)

// 调试日志 - 青色
#define LOG_D(tag, fmt, ...) \
    log_print(LOG_LEVEL_DEBUG, tag, LOG_COLOR_CYAN "[D]" LOG_COLOR_RESET " %-*s| " fmt, LOG_TAG_WIDTH, tag, ##__VA_ARGS__)

// 详细日志
#define LOG_V(tag, fmt, ...) \
    log_print(LOG_LEVEL_VERBOSE, tag, "[V] %-*s| " fmt, LOG_TAG_WIDTH, tag, ##__VA_ARGS__)

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

// ============================================================================
//                          自动对齐输出宏
// ============================================================================

// 对齐配置
#define LOG_ALIGN_TAG_WIDTH 8   // 标签对齐宽度 (如 [INFO], [ERROR])
#define LOG_ALIGN_FILE_WIDTH 20 // 文件名对齐宽度
#define LOG_ALIGN_LINE_WIDTH 4  // 行号对齐宽度
#define LOG_ALIGN_FUNC_WIDTH 20 // 函数名对齐宽度
#define LOG_ALIGN_KEY_WIDTH 16  // 键值对的键对齐宽度

// ------------ 获取短文件名（去除路径） ------------
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__))

// ------------ 带对齐的日志宏（简洁格式）------------
// 格式: [TAG    ] message
#define out_log(fmt, ...) \
    printf(FG_MAGENTA "%-8s" RESET_ALL " " fmt "\n", "[LOG]", ##__VA_ARGS__)
#define error(fmt, ...) \
    printf(FG_RED "%-8s" RESET_ALL " " fmt "\n", "[ERROR]", ##__VA_ARGS__)
#define warning(fmt, ...) \
    printf(FG_YELLOW "%-8s" RESET_ALL " " fmt "\n", "[WARN]", ##__VA_ARGS__)
#define info(fmt, ...) \
    printf(FG_GREEN "%-8s" RESET_ALL " " fmt "\n", "[INFO]", ##__VA_ARGS__)
#define debug(fmt, ...) \
    printf(FG_BLUE "%-8s" RESET_ALL " " fmt "\n", "[DEBUG]", ##__VA_ARGS__)
#define success(fmt, ...) \
    printf(FG_CYAN "%-8s" RESET_ALL " " fmt "\n", "[OK]", ##__VA_ARGS__)

// ------------ 带文件位置的详细日志宏（调试用）------------
// 格式: [TAG    ] filename.c:123  | func_name            | message
#define out_log_v(fmt, ...)                                               \
    printf(FG_MAGENTA "%-8s" RESET_ALL " %-20s:%-4d | %-20s | " fmt "\n", \
           "[LOG]", __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define error_v(fmt, ...)                                             \
    printf(FG_RED "%-8s" RESET_ALL " %-20s:%-4d | %-20s | " fmt "\n", \
           "[ERROR]", __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define warning_v(fmt, ...)                                              \
    printf(FG_YELLOW "%-8s" RESET_ALL " %-20s:%-4d | %-20s | " fmt "\n", \
           "[WARN]", __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define info_v(fmt, ...)                                                \
    printf(FG_GREEN "%-8s" RESET_ALL " %-20s:%-4d | %-20s | " fmt "\n", \
           "[INFO]", __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define debug_v(fmt, ...)                                              \
    printf(FG_BLUE "%-8s" RESET_ALL " %-20s:%-4d | %-20s | " fmt "\n", \
           "[DEBUG]", __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define success_v(fmt, ...)                                            \
    printf(FG_CYAN "%-8s" RESET_ALL " %-20s:%-4d | %-20s | " fmt "\n", \
           "[OK]", __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

// ------------ 键值对输出宏（自动对齐）------------
// 格式: Key              : Value
#define log_kv(key, fmt, ...) \
    printf("%-*s : " fmt "\n", LOG_ALIGN_KEY_WIDTH, key, ##__VA_ARGS__)

// 带颜色的键值对
#define log_kv_info(key, fmt, ...) \
    printf(FG_GREEN "%-*s" RESET_ALL " : " fmt "\n", LOG_ALIGN_KEY_WIDTH, key, ##__VA_ARGS__)
#define log_kv_warn(key, fmt, ...) \
    printf(FG_YELLOW "%-*s" RESET_ALL " : " fmt "\n", LOG_ALIGN_KEY_WIDTH, key, ##__VA_ARGS__)
#define log_kv_error(key, fmt, ...) \
    printf(FG_RED "%-*s" RESET_ALL " : " fmt "\n", LOG_ALIGN_KEY_WIDTH, key, ##__VA_ARGS__)

// ------------ 表格输出辅助宏 ------------
// 打印分隔线
#define log_separator(width, ch)          \
    do                                    \
    {                                     \
        for (int i = 0; i < (width); i++) \
            putchar(ch);                  \
        putchar('\n');                    \
    } while (0)

// 打印表头（自动居中）
#define log_table_header(fmt, ...) \
    printf(TEXT_BOLD fmt RESET_ALL "\n", ##__VA_ARGS__)

// ------------ 进度/状态指示宏 ------------
// 打印带状态的行
#define log_status(name, status) \
    printf("%-*s [%s]\n", LOG_ALIGN_KEY_WIDTH, name, status)

#define log_status_ok(name) \
    printf("%-*s [" FG_GREEN "OK" RESET_ALL "]\n", LOG_ALIGN_KEY_WIDTH, name)
#define log_status_fail(name) \
    printf("%-*s [" FG_RED "FAIL" RESET_ALL "]\n", LOG_ALIGN_KEY_WIDTH, name)
#define log_status_skip(name) \
    printf("%-*s [" FG_YELLOW "SKIP" RESET_ALL "]\n", LOG_ALIGN_KEY_WIDTH, name)

#endif /* __DF_LOG_H__ */
