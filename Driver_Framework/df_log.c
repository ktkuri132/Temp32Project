/**
 * @file df_log.c
 * @brief 驱动框架统一日志系统实现
 * @author Driver Framework Team
 * @date 2026-01-01
 */

#include "df_log.h"
#include "df_uart.h"
#include "df_init.h"
#include <string.h>
#include <stdlib.h>

// ============ UART 设备绑定 ============
static df_uart_t *g_log_uart = NULL;

// ============ 时间戳回调函数（需要用户实现）============
static uint32_t (*g_get_tick_func)(void) = NULL;

// ============ 日志缓冲区 ============
typedef struct
{
    char *buffer;     // 缓冲区指针
    size_t size;      // 缓冲区总大小
    size_t write_pos; // 写位置
    size_t read_pos;  // 读位置
    size_t used;      // 已使用大小
    bool initialized; // 是否已初始化
} log_buffer_t;

static log_buffer_t g_log_buffer = {
    .buffer = NULL,
    .size = 0,
    .write_pos = 0,
    .read_pos = 0,
    .used = 0,
    .initialized = false};

// ============ 全局日志配置 ============
log_config_t g_log_config = {
    .level = LOG_LEVEL_INFO,
    .enable_timestamp = false,
    .enable_color = false,
    .output_func = NULL,
    .buffer_mode = LOG_BUFFER_MODE_BUFFERED,
    .overflow_policy = LOG_OVERFLOW_OVERWRITE};

// ============ 日志初始化 ============
void log_init(log_level_t level)
{
    g_log_config.level = level;
    g_log_config.enable_timestamp = false;
    g_log_config.enable_color = true;
    g_log_config.output_func = NULL;
    g_log_config.buffer_mode = LOG_BUFFER_MODE_BUFFERED;
    g_log_config.overflow_policy = LOG_OVERFLOW_OVERWRITE;

    // 默认初始化1KB缓冲区
    if (!g_log_buffer.initialized)
    {
        log_buffer_init(1024);
    }
}

void log_set_level(log_level_t level)
{
    g_log_config.level = level;
}

void log_set_output(void (*func)(const char *))
{
    g_log_config.output_func = func;
}

void log_set_timestamp_func(uint32_t (*get_tick)(void))
{
    g_get_tick_func = get_tick;
}

void log_enable_timestamp(bool enable)
{
    g_log_config.enable_timestamp = enable;
}

// ============ UART 设备绑定 ============
/**
 * @brief 绑定 UART 设备用于日志输出
 * @param uart UART 设备指针
 * @note 绑定后，所有日志将通过该 UART 输出
 */
void log_set_uart(struct df_uart_struct *uart)
{
    g_log_uart = uart;

    // 同时设置输出函数为 UART 发送
    if (uart != NULL && uart->send != NULL)
    {
        // 使用 lambda 风格的包装函数
        g_log_config.output_func = NULL; // 清除旧的回调
    }
}

// ============ 统一输出函数 ============
/**
 * @brief 内部统一输出函数
 * @param str 要输出的字符串
 */
static void log_output_internal(const char *str)
{
    // 优先使用绑定的 UART 设备
    if (g_log_uart != NULL && g_log_uart->send != NULL)
    {
        g_log_uart->send(arg_ptr((void *)str));
    }
    // 其次使用自定义输出函数
    else if (g_log_config.output_func != NULL)
    {
        g_log_config.output_func(str);
    }
    // 最后回退到标准输出
    else
    {
        printf("%s", str);
        fflush(stdout);
    }
}

/**
 * @brief 原始字符串输出（不带格式化）
 * @param str 要输出的字符串
 */
void log_raw(const char *str)
{
    if (str != NULL)
    {
        log_output_internal(str);
    }
}

/**
 * @brief 格式化输出（不带日志级别/标签）
 * @param fmt 格式字符串
 * @param ... 可变参数
 */
void log_printf(const char *fmt, ...)
{
    static char log_printf_buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(log_printf_buf, sizeof(log_printf_buf), fmt, args);
    va_end(args);
    log_output_internal(log_printf_buf);
}

// ============ 缓冲区管理 ============
void log_buffer_init(size_t size)
{
    // 如果已经初始化，先释放
    if (g_log_buffer.initialized && g_log_buffer.buffer != NULL)
    {
        free(g_log_buffer.buffer);
    }

    // 分配新缓冲区
    g_log_buffer.buffer = (char *)malloc(size);
    if (g_log_buffer.buffer != NULL)
    {
        g_log_buffer.size = size;
        g_log_buffer.write_pos = 0;
        g_log_buffer.read_pos = 0;
        g_log_buffer.used = 0;
        g_log_buffer.initialized = true;
    }
}

void log_set_buffer_mode(log_buffer_mode_t mode)
{
    g_log_config.buffer_mode = mode;
}

void log_set_overflow_policy(log_overflow_policy_t policy)
{
    g_log_config.overflow_policy = policy;
}

void log_buffer_clear(void)
{
    if (g_log_buffer.initialized)
    {
        g_log_buffer.write_pos = 0;
        g_log_buffer.read_pos = 0;
        g_log_buffer.used = 0;
    }
}

size_t log_buffer_get_usage(void)
{
    return g_log_buffer.used;
}

bool log_buffer_is_full(void)
{
    return g_log_buffer.used >= g_log_buffer.size;
}

// ============ 内部函数：写入缓冲区 ============
static int log_buffer_write(const char *data, size_t len)
{
    if (!g_log_buffer.initialized || g_log_buffer.buffer == NULL)
    {
        return -1;
    }

    // 检查是否有足够空间
    if (g_log_buffer.used + len > g_log_buffer.size)
    {
        // 根据策略处理溢出
        if (g_log_config.overflow_policy == LOG_OVERFLOW_DISCARD)
        {
            return -1; // 丢弃新数据
        }
        // LOG_OVERFLOW_OVERWRITE: 覆盖旧数据（环形缓冲区）
    }

    // 写入数据
    for (size_t i = 0; i < len; i++)
    {
        g_log_buffer.buffer[g_log_buffer.write_pos] = data[i];
        g_log_buffer.write_pos = (g_log_buffer.write_pos + 1) % g_log_buffer.size;

        if (g_log_buffer.used < g_log_buffer.size)
        {
            g_log_buffer.used++;
        }
        else
        {
            // 覆盖模式：移动读指针
            g_log_buffer.read_pos = (g_log_buffer.read_pos + 1) % g_log_buffer.size;
        }
    }

    return 0;
}

// ============ 刷新缓冲区 ============
int log_flush(void)
{
    if (!g_log_buffer.initialized || g_log_buffer.used == 0)
    {
        return 0;
    }

    // 临时缓冲区用于输出
    char temp[256];
    size_t output_count = 0;

    while (g_log_buffer.used > 0)
    {
        size_t chunk_size = (g_log_buffer.used < sizeof(temp)) ? g_log_buffer.used : sizeof(temp);

        // 从缓冲区读取数据
        for (size_t i = 0; i < chunk_size; i++)
        {
            temp[i] = g_log_buffer.buffer[g_log_buffer.read_pos];
            g_log_buffer.read_pos = (g_log_buffer.read_pos + 1) % g_log_buffer.size;
            g_log_buffer.used--;
        }

        // 输出数据
        temp[chunk_size] = '\0';
        log_output_internal(temp);

        output_count += chunk_size;
    }

    return output_count;
}

// ============ 日志打印实现 ============
// 时间戳宽度配置（用于对齐）
#define LOG_TIMESTAMP_WIDTH 8

void log_print(log_level_t level, const char *tag, const char *fmt, ...)
{
    // 检查日志级别
    if (level > g_log_config.level)
    {
        return;
    }

    // 构建日志前缀（包含时间戳）
    char prefix[64] = {0};

    // 添加时间戳（固定宽度对齐）
    if (g_log_config.enable_timestamp && g_get_tick_func != NULL)
    {
        uint32_t timestamp = g_get_tick_func();
        snprintf(prefix, sizeof(prefix), "[%*lu] ", LOG_TIMESTAMP_WIDTH, (unsigned long)timestamp);
    }

    // 格式化日志内容
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    // 组合完整日志（添加换行符）
    char full_log[384];
    snprintf(full_log, sizeof(full_log), "%s%s\n", prefix, buffer);

    // 根据模式选择输出方式
    if (g_log_config.buffer_mode == LOG_BUFFER_MODE_BUFFERED)
    {
        // 缓冲模式：写入缓冲区
        log_buffer_write(full_log, strlen(full_log));
    }
    else
    {
        // 直接输出模式：使用统一输出函数
        log_output_internal(full_log);
    }
}

// ============ 十六进制数据打印 ============
void log_hex_dump(log_level_t level, const char *tag, const void *data, size_t len)
{
    if (level > g_log_config.level || data == NULL || len == 0)
    {
        return;
    }

    const uint8_t *bytes = (const uint8_t *)data;
    char hex_str[16 * 3 + 1]; // 每行最多16字节

    log_print(level, tag, "HEX Dump (%d bytes):\n", len);

    for (size_t i = 0; i < len; i += 16)
    {
        size_t line_len = (len - i) > 16 ? 16 : (len - i);
        int offset = 0;

        // 转换为十六进制字符串
        for (size_t j = 0; j < line_len; j++)
        {
            offset += snprintf(hex_str + offset, sizeof(hex_str) - offset,
                               "%02X ", bytes[i + j]);
        }

        log_print(level, tag, "  %04X: %s\n", (unsigned int)i, hex_str);
    }
}

// ============ 自动初始化 ============
/**
 * @brief 日志系统自动初始化函数
 * @details 在框架初始化时自动调用，初始化日志系统为INFO级别
 * @return 0表示成功
 */
int df_log_init(void)
{
    log_init(LOG_LEVEL_INFO);
    log_print(LOG_LEVEL_INFO, "", CLEAR_SCREEN);
    log_print(LOG_LEVEL_INFO, "", CURSOR_HOME);
    return 0;
}
