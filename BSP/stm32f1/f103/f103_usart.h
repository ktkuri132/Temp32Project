/**
 * @file f103_usart.h
 * @brief STM32F103系列USART公共驱动头文件
 * @details 适用于所有STM32F103封装
 */

#ifndef __F103_USART_H
#define __F103_USART_H

#include <stm32f10x.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /*===========================================================================*/
    /*                              类型定义                                      */
    /*===========================================================================*/

    /**
     * @brief USART端口枚举
     */
    typedef enum
    {
        F103_USART1 = 0,
        F103_USART2,
        F103_USART3,
        F103_USART_MAX
    } f103_usart_port_t;

    /**
     * @brief USART配置结构体
     */
    typedef struct
    {
        f103_usart_port_t port; /**< USART端口 */
        uint32_t baudrate;      /**< 波特率 */
        uint8_t word_length;    /**< 数据位 8/9 */
        uint8_t stop_bits;      /**< 停止位 1/2 */
        uint8_t parity;         /**< 校验位 0:无 1:奇 2:偶 */
    } f103_usart_config_t;

    /*===========================================================================*/
    /*                              API函数声明                                   */
    /*===========================================================================*/

    /**
     * @brief 初始化USART
     * @param config 配置结构体
     * @return 0成功，-1失败
     */
    int f103_usart_init(const f103_usart_config_t *config);

    /**
     * @brief 快速初始化USART (8N1)
     * @param port USART端口
     * @param baudrate 波特率
     * @return 0成功，-1失败
     */
    int f103_usart_init_quick(f103_usart_port_t port, uint32_t baudrate);

    /**
     * @brief 发送单个字符
     * @param port USART端口
     * @param ch 字符
     */
    void f103_usart_send_char(f103_usart_port_t port, uint8_t ch);

    /**
     * @brief 发送字符串
     * @param port USART端口
     * @param str 字符串
     */
    void f103_usart_send_string(f103_usart_port_t port, const char *str);

    /**
     * @brief 发送数据
     * @param port USART端口
     * @param data 数据指针
     * @param len 数据长度
     */
    void f103_usart_send_data(f103_usart_port_t port, const uint8_t *data, uint32_t len);

    /**
     * @brief 接收单个字符 (阻塞)
     * @param port USART端口
     * @return 接收到的字符
     */
    uint8_t f103_usart_recv_char(f103_usart_port_t port);

    /**
     * @brief 检查是否有数据可读
     * @param port USART端口
     * @return true有数据，false无数据
     */
    bool f103_usart_available(f103_usart_port_t port);

#ifdef __cplusplus
}
#endif

#endif /* __F103_USART_H */
