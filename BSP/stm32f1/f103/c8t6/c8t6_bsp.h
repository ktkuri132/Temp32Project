/**
 * @file c8t6_bsp.h
 * @brief STM32F103C8T6 BSP头文件
 * @note 包含芯片配置和F103驱动
 */

#ifndef __C8T6_BSP_H
#define __C8T6_BSP_H

#ifdef __cplusplus
extern "C"
{
#endif

/* 芯片配置 */
#include "c8t6_config.h"

/* F103系列驱动 */
#include "../f103_bsp.h"

    /**
     * @brief C8T6 板级初始化
     * @note 初始化系统时钟、默认GPIO、调试串口等
     */
    void c8t6_board_init(void);

    /**
     * @brief 初始化板载LED
     */
    void c8t6_led_init(void);

    /**
     * @brief 控制板载LED
     * @param on true:点亮 false:熄灭
     */
    void c8t6_led_set(bool on);

    /**
     * @brief 翻转板载LED
     */
    void c8t6_led_toggle(void);

    /**
     * @brief 初始化调试串口
     */
    void c8t6_debug_uart_init(void);

    /**
     * @brief 调试串口发送字符
     * @param c 字符
     */
    void c8t6_debug_putchar(char c);

    /**
     * @brief 调试串口发送字符串
     * @param str 字符串
     */
    void c8t6_debug_puts(const char *str);

#ifdef __cplusplus
}
#endif

#endif /* __C8T6_BSP_H */
