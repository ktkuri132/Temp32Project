/**
 * @file df_log.c
 * @brief 驱动框架统一日志系统实现
 * @author Driver Framework Team
 * @date 2026-01-01
 */

#include "df_log.h"
#include "df_init.h"
#include <string.h>
#include <stdlib.h>

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
    .buffer_mode = LOG_BUFFER_MODE_DIRECT,
    .overflow_policy = LOG_OVERFLOW_OVERWRITE};

// ============ 日志初始化 ============
void log_init(log_level_t level)
{
    g_log_config.level = level;
    g_log_config.enable_timestamp = false;
    g_log_config.enable_color = false;
    g_log_config.output_func = NULL;
    g_log_config.buffer_mode = LOG_BUFFER_MODE_DIRECT;
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
        if (g_log_config.output_func)
        {
            temp[chunk_size] = '\0';
            g_log_config.output_func(temp);
        }
        else
        {
            fwrite(temp, 1, chunk_size, stdout);
            fflush(stdout);
        }

        output_count += chunk_size;
    }

    return output_count;
}

// ============ 日志打印实现 ============
void log_print(log_level_t level, const char *tag, const char *fmt, ...)
{
    // 检查日志级别
    if (level > g_log_config.level)
    {
        return;
    }

    // 构建日志前缀（包含时间戳）
    char prefix[96] = {0};
    int offset = 0;

    // 添加时间戳
    if (g_log_config.enable_timestamp && g_get_tick_func != NULL)
    {
        uint32_t timestamp = g_get_tick_func();
        offset = snprintf(prefix, sizeof(prefix), "[%lu] ", (unsigned long)timestamp);
    }

    // 添加标签
    if (tag && tag[0] != '\0')
    {
        snprintf(prefix + offset, sizeof(prefix) - offset, "[%s] ", tag);
    }

    // 格式化日志内容
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    // 组合完整日志
    char full_log[384];
    snprintf(full_log, sizeof(full_log), "%s%s", prefix, buffer);

    // 根据模式选择输出方式
    if (g_log_config.buffer_mode == LOG_BUFFER_MODE_BUFFERED)
    {
        // 缓冲模式：写入缓冲区
        log_buffer_write(full_log, strlen(full_log));
    }
    else
    {
        // 直接输出模式
        if (g_log_config.output_func)
        {
            g_log_config.output_func(full_log);
        }
        else
        {
            printf("%s", full_log);
        }
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
static int df_log_auto_init(void)
{
    log_init(LOG_LEVEL_INFO);
    LOG_I("LOG", "Log system initialized\n");
    return 0;
}

// 将日志初始化注册到BOARD级别（最先初始化）
DF_INIT_EXPORT(df_log_auto_init, DF_INIT_EXPORT_BOARD);
