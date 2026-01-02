/**
 * @file usart.c
 * @brief STM32F4 USART 片上外设驱动
 * @note 使用 f407_usart 底层驱动实现 df_uart.h 接口
 * @note 片上外设通过 DF_BOARD_INIT 分散加载初始化
 */

#include "driver.h"
#include "df_uart.h"
#include "df_init.h"
#include "df_log.h"
#include "f407_usart.h"
#include <stdarg.h>

/*============================ 内部变量 ============================*/
static f407_usart_handle_t usart1_handle;

/*============================ 前向声明 ============================*/
int usart1_init(df_arg_t arg);
int usart1_deinit(df_arg_t arg);
int usart1_send(df_arg_t arg);
int usart1_receive(df_arg_t arg);
static int usart1_printf(const char *format, ...);

/*============================ 设备实例 ============================*/
df_uart_t Debug = {
    .init_flag = false,
    .num = 1,
    .name = DEBUG_UART_NAME,
    .baudrate = 115200,
    .init = usart1_init,
    .deinit = usart1_deinit,
    .send = usart1_send,
    .printf = usart1_printf,
    .receive = usart1_receive,
    .send_dma = NULL,
    .receive_dma = NULL};

/*============================ 接口实现 ============================*/

/**
 * @brief 初始化 USART1
 */
int usart1_init(df_arg_t arg)
{
    (void)arg;

    /* 使用 f407 驱动初始化 USART1 */
    if (f407_usart_init_simple(&usart1_handle, F407_USART1,
                               Debug.baudrate, F407_USART1_PINS_PA9_PA10) != 0)
    {
        return -1;
    }

    /* 使能接收中断 */
    f407_usart_enable_rx_irq(&usart1_handle, 3);

    Debug.init_flag = true;

    return 0;
}

/**
 * @brief 关闭 USART1
 */
int usart1_deinit(df_arg_t arg)
{
    (void)arg;

    /* 反初始化 USART1 */
    f407_usart_deinit(&usart1_handle);
    Debug.init_flag = false;

    return 0;
}

/**
 * @brief 发送字符串
 */
int usart1_send(df_arg_t arg)
{
    if (arg.ptr == NULL)
        return -1;

    f407_usart_send_string(&usart1_handle, (const char *)arg.ptr);
    return 0;
}

/**
 * @brief 接收数据
 */
int usart1_receive(df_arg_t arg)
{
    uint8_t *data = (uint8_t *)arg.ptr;
    if (data == NULL)
        return -1;

    return f407_usart_receive_byte(&usart1_handle, data, F407_USART_TIMEOUT);
}

/**
 * @brief 格式化输出
 */
static int usart1_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    char buffer[128];
    int len = vsnprintf(buffer, sizeof(buffer), format, args);

    va_end(args);

    f407_usart_send_string(&usart1_handle, buffer);
    return len;
}

/*============================ 标准输出重定向 ============================*/

/**
 * @brief 重定向 putchar (用于 printf)
 */
int __io_putchar(int ch)
{
    f407_usart_send_byte(&usart1_handle, (uint8_t)ch);
    return ch;
}

#ifndef __clang__
/**
 * @brief printf 实现
 */
int printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    char buffer[128];
    int len = vsnprintf(buffer, sizeof(buffer), format, args);

    va_end(args);

    f407_usart_send_string(&usart1_handle, buffer);
    return len;
}
#endif

/*============================ 辅助函数 ============================*/

/**
 * @brief 用于日志系统的发送函数
 */
static void usart1_log_send(const char *data)
{
    f407_usart_send_string(&usart1_handle, data);
}

/*============================ 片上外设自动初始化 ============================*/

/**
 * @brief USART1 自动初始化函数
 * @note 通过 DF_BOARD_INIT 宏在系统启动时自动调用
 */
static int usart1_auto_init(void)
{
    int ret = usart1_init(arg_null);
    if (ret == 0)
    {
        g_log_config.output_func = usart1_send;
        LOG_I("USART1", "USART1 initialized with baud rate %d", Debug.baudrate);
    }
    return ret;
}
DF_BOARD_INIT(usart1_auto_init);
