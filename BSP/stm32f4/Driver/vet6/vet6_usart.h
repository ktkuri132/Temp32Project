/**
 * @file vet6_usart.h
 * @brief STM32F407VET6 USART板级驱动头文件
 * @details 提供USART1-USART6的初始化和操作API
 *          - USART1, USART6: APB2 (最高84MHz)
 *          - USART2, USART3, UART4, UART5: APB1 (最高42MHz)
 *
 * 默认引脚配置:
 *   USART1: PA9(TX), PA10(RX) 或 PB6(TX), PB7(RX)
 *   USART2: PA2(TX), PA3(RX) 或 PD5(TX), PD6(RX)
 *   USART3: PB10(TX), PB11(RX) 或 PC10(TX), PC11(RX) 或 PD8(TX), PD9(RX)
 *   UART4:  PA0(TX), PA1(RX) 或 PC10(TX), PC11(RX)
 *   UART5:  PC12(TX), PD2(RX)
 *   USART6: PC6(TX), PC7(RX)
 */

#ifndef __VET6_USART_H
#define __VET6_USART_H

#include <stm32f4xx.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include "vet6_gpio.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /*===========================================================================*/
    /*                              类型定义                                      */
    /*===========================================================================*/

    /**
     * @brief USART编号枚举
     */
    typedef enum
    {
        VET6_USART1 = 0,
        VET6_USART2,
        VET6_USART3,
        VET6_UART4,
        VET6_UART5,
        VET6_USART6,
        VET6_USART_MAX
    } vet6_usart_t;

    /**
     * @brief USART数据位
     */
    typedef enum
    {
        VET6_USART_WORDLEN_8B = 0,
        VET6_USART_WORDLEN_9B = 1
    } vet6_usart_wordlen_t;

    /**
     * @brief USART停止位
     */
    typedef enum
    {
        VET6_USART_STOPBITS_1 = 0,
        VET6_USART_STOPBITS_0_5 = 1,
        VET6_USART_STOPBITS_2 = 2,
        VET6_USART_STOPBITS_1_5 = 3
    } vet6_usart_stopbits_t;

    /**
     * @brief USART校验位
     */
    typedef enum
    {
        VET6_USART_PARITY_NONE = 0,
        VET6_USART_PARITY_EVEN = 1,
        VET6_USART_PARITY_ODD = 2
    } vet6_usart_parity_t;

    /**
     * @brief USART模式
     */
    typedef enum
    {
        VET6_USART_MODE_RX = 0x01,
        VET6_USART_MODE_TX = 0x02,
        VET6_USART_MODE_TX_RX = 0x03
    } vet6_usart_mode_t;

    /**
     * @brief USART硬件流控
     */
    typedef enum
    {
        VET6_USART_HWFLOW_NONE = 0,
        VET6_USART_HWFLOW_RTS = 1,
        VET6_USART_HWFLOW_CTS = 2,
        VET6_USART_HWFLOW_RTS_CTS = 3
    } vet6_usart_hwflow_t;

    /**
     * @brief USART引脚映射
     */
    typedef enum
    {
        VET6_USART1_PINS_PA9_PA10 = 0, // USART1: PA9(TX), PA10(RX)
        VET6_USART1_PINS_PB6_PB7,      // USART1: PB6(TX), PB7(RX)
        VET6_USART2_PINS_PA2_PA3,      // USART2: PA2(TX), PA3(RX)
        VET6_USART2_PINS_PD5_PD6,      // USART2: PD5(TX), PD6(RX)
        VET6_USART3_PINS_PB10_PB11,    // USART3: PB10(TX), PB11(RX)
        VET6_USART3_PINS_PC10_PC11,    // USART3: PC10(TX), PC11(RX)
        VET6_USART3_PINS_PD8_PD9,      // USART3: PD8(TX), PD9(RX)
        VET6_UART4_PINS_PA0_PA1,       // UART4: PA0(TX), PA1(RX)
        VET6_UART4_PINS_PC10_PC11,     // UART4: PC10(TX), PC11(RX)
        VET6_UART5_PINS_PC12_PD2,      // UART5: PC12(TX), PD2(RX)
        VET6_USART6_PINS_PC6_PC7       // USART6: PC6(TX), PC7(RX)
    } vet6_usart_pinmap_t;

    /**
     * @brief USART配置结构体
     */
    typedef struct
    {
        vet6_usart_t usart;             // USART编号
        uint32_t baudrate;              // 波特率
        vet6_usart_wordlen_t wordlen;   // 数据位
        vet6_usart_stopbits_t stopbits; // 停止位
        vet6_usart_parity_t parity;     // 校验位
        vet6_usart_mode_t mode;         // 工作模式
        vet6_usart_hwflow_t hwflow;     // 硬件流控
        vet6_usart_pinmap_t pinmap;     // 引脚映射
    } vet6_usart_config_t;

    /**
     * @brief USART接收回调函数类型
     */
    typedef void (*vet6_usart_rx_callback_t)(uint8_t data);

    /**
     * @brief USART句柄结构体
     */
    typedef struct
    {
        USART_TypeDef *instance;              // USART外设指针
        vet6_usart_config_t config;           // 配置信息
        bool initialized;                     // 初始化标志
        vet6_usart_rx_callback_t rx_callback; // 接收回调

        /* 接收缓冲区 */
        uint8_t *rx_buffer;        // 接收缓冲区指针
        uint16_t rx_size;          // 缓冲区大小
        volatile uint16_t rx_head; // 环形缓冲区头指针
        volatile uint16_t rx_tail; // 环形缓冲区尾指针
    } vet6_usart_handle_t;

/*===========================================================================*/
/*                              宏定义                                        */
/*===========================================================================*/

/* USART时钟频率 */
#define VET6_USART1_CLK 84000000UL // APB2时钟
#define VET6_USART6_CLK 84000000UL // APB2时钟
#define VET6_USART2_CLK 42000000UL // APB1时钟
#define VET6_USART3_CLK 42000000UL // APB1时钟
#define VET6_UART4_CLK 42000000UL  // APB1时钟
#define VET6_UART5_CLK 42000000UL  // APB1时钟

/* 常用波特率 */
#define VET6_USART_BAUD_9600 9600
#define VET6_USART_BAUD_19200 19200
#define VET6_USART_BAUD_38400 38400
#define VET6_USART_BAUD_57600 57600
#define VET6_USART_BAUD_115200 115200
#define VET6_USART_BAUD_230400 230400
#define VET6_USART_BAUD_460800 460800
#define VET6_USART_BAUD_921600 921600

/* 超时时间 */
#define VET6_USART_TIMEOUT 10000

    /*===========================================================================*/
    /*                              初始化API                                     */
    /*===========================================================================*/

    /**
     * @brief 使能USART时钟
     * @param usart USART编号
     */
    void vet6_usart_clk_enable(vet6_usart_t usart);

    /**
     * @brief 禁用USART时钟
     * @param usart USART编号
     */
    void vet6_usart_clk_disable(vet6_usart_t usart);

    /**
     * @brief 获取USART外设基地址
     * @param usart USART编号
     * @return USART外设指针
     */
    USART_TypeDef *vet6_usart_get_base(vet6_usart_t usart);

    /**
     * @brief 初始化USART引脚
     * @param pinmap 引脚映射
     * @return 0:成功, -1:失败
     */
    int vet6_usart_pins_init(vet6_usart_pinmap_t pinmap);

    /**
     * @brief 初始化USART
     * @param handle USART句柄指针
     * @param config 配置结构体指针
     * @return 0:成功, -1:失败
     */
    int vet6_usart_init(vet6_usart_handle_t *handle, const vet6_usart_config_t *config);

    /**
     * @brief 快速初始化USART (8N1)
     * @param handle USART句柄指针
     * @param usart USART编号
     * @param baudrate 波特率
     * @param pinmap 引脚映射
     * @return 0:成功, -1:失败
     */
    int vet6_usart_init_simple(vet6_usart_handle_t *handle, vet6_usart_t usart,
                               uint32_t baudrate, vet6_usart_pinmap_t pinmap);

    /**
     * @brief 反初始化USART
     * @param handle USART句柄指针
     */
    void vet6_usart_deinit(vet6_usart_handle_t *handle);

    /*===========================================================================*/
    /*                              基本操作API                                   */
    /*===========================================================================*/

    /**
     * @brief 使能USART
     * @param handle USART句柄指针
     */
    void vet6_usart_enable(vet6_usart_handle_t *handle);

    /**
     * @brief 禁用USART
     * @param handle USART句柄指针
     */
    void vet6_usart_disable(vet6_usart_handle_t *handle);

    /**
     * @brief 发送单字节
     * @param handle USART句柄指针
     * @param data 发送数据
     * @return 0:成功, -1:失败
     */
    int vet6_usart_send_byte(vet6_usart_handle_t *handle, uint8_t data);

    /**
     * @brief 接收单字节(阻塞)
     * @param handle USART句柄指针
     * @param data 接收数据指针
     * @param timeout 超时时间
     * @return 0:成功, -1:超时
     */
    int vet6_usart_receive_byte(vet6_usart_handle_t *handle, uint8_t *data, uint32_t timeout);

    /**
     * @brief 发送数据缓冲区
     * @param handle USART句柄指针
     * @param data 发送数据指针
     * @param len 数据长度
     * @return 0:成功, -1:失败
     */
    int vet6_usart_send(vet6_usart_handle_t *handle, const uint8_t *data, uint16_t len);

    /**
     * @brief 发送字符串
     * @param handle USART句柄指针
     * @param str 字符串指针
     * @return 0:成功, -1:失败
     */
    int vet6_usart_send_string(vet6_usart_handle_t *handle, const char *str);

    /**
     * @brief 接收数据到缓冲区(阻塞)
     * @param handle USART句柄指针
     * @param data 接收数据指针
     * @param len 数据长度
     * @param timeout 超时时间
     * @return 实际接收长度
     */
    int vet6_usart_receive(vet6_usart_handle_t *handle, uint8_t *data, uint16_t len, uint32_t timeout);

    /**
     * @brief 格式化打印
     * @param handle USART句柄指针
     * @param format 格式字符串
     * @return 发送的字符数
     */
    int vet6_usart_printf(vet6_usart_handle_t *handle, const char *format, ...);

    /*===========================================================================*/
    /*                              中断相关API                                   */
    /*===========================================================================*/

    /**
     * @brief 使能USART接收中断
     * @param handle USART句柄指针
     * @param priority 中断优先级
     */
    void vet6_usart_enable_rx_irq(vet6_usart_handle_t *handle, uint8_t priority);

    /**
     * @brief 禁用USART接收中断
     * @param handle USART句柄指针
     */
    void vet6_usart_disable_rx_irq(vet6_usart_handle_t *handle);

    /**
     * @brief 设置接收回调函数
     * @param handle USART句柄指针
     * @param callback 回调函数
     */
    void vet6_usart_set_rx_callback(vet6_usart_handle_t *handle, vet6_usart_rx_callback_t callback);

    /**
     * @brief 设置接收缓冲区
     * @param handle USART句柄指针
     * @param buffer 缓冲区指针
     * @param size 缓冲区大小
     */
    void vet6_usart_set_rx_buffer(vet6_usart_handle_t *handle, uint8_t *buffer, uint16_t size);

    /**
     * @brief 从接收缓冲区读取数据
     * @param handle USART句柄指针
     * @param data 接收数据指针
     * @param len 最大读取长度
     * @return 实际读取长度
     */
    int vet6_usart_read_buffer(vet6_usart_handle_t *handle, uint8_t *data, uint16_t len);

    /**
     * @brief 获取接收缓冲区数据量
     * @param handle USART句柄指针
     * @return 可读取数据量
     */
    uint16_t vet6_usart_available(vet6_usart_handle_t *handle);

    /**
     * @brief 清空接收缓冲区
     * @param handle USART句柄指针
     */
    void vet6_usart_flush_rx(vet6_usart_handle_t *handle);

    /**
     * @brief USART中断处理(需在中断服务函数中调用)
     * @param handle USART句柄指针
     */
    void vet6_usart_irq_handler(vet6_usart_handle_t *handle);

    /*===========================================================================*/
    /*                              辅助功能API                                   */
    /*===========================================================================*/

    /**
     * @brief 获取USART时钟频率
     * @param usart USART编号
     * @return 时钟频率(Hz)
     */
    uint32_t vet6_usart_get_clk(vet6_usart_t usart);

    /**
     * @brief 设置波特率
     * @param handle USART句柄指针
     * @param baudrate 波特率
     */
    void vet6_usart_set_baudrate(vet6_usart_handle_t *handle, uint32_t baudrate);

    /**
     * @brief 检查发送是否完成
     * @param handle USART句柄指针
     * @return true:完成, false:未完成
     */
    bool vet6_usart_tx_complete(vet6_usart_handle_t *handle);

    /**
     * @brief 检查是否有数据可接收
     * @param handle USART句柄指针
     * @return true:有数据, false:无数据
     */
    bool vet6_usart_rx_ready(vet6_usart_handle_t *handle);

    /**
     * @brief 等待发送完成
     * @param handle USART句柄指针
     * @param timeout 超时时间
     * @return 0:成功, -1:超时
     */
    int vet6_usart_wait_tx_complete(vet6_usart_handle_t *handle, uint32_t timeout);

    /**
     * @brief 清除错误标志
     * @param handle USART句柄指针
     */
    void vet6_usart_clear_errors(vet6_usart_handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* __VET6_USART_H */
