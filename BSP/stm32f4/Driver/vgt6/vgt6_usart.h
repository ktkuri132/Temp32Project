/**
 * @file vgt6_usart.h
 * @brief STM32F407VGT6 USART驱动 - 复用VET6驱动
 */

#ifndef __VGT6_USART_H
#define __VGT6_USART_H

#include "../vet6/vet6_usart.h"

/* 类型别名 */
typedef vet6_usart_t vgt6_usart_t;
typedef vet6_usart_parity_t vgt6_usart_parity_t;
typedef vet6_usart_stopbits_t vgt6_usart_stopbits_t;
typedef vet6_usart_config_t vgt6_usart_config_t;
typedef vet6_usart_handle_t vgt6_usart_handle_t;

/* 枚举别名 */
#define VGT6_USART1 VET6_USART1
#define VGT6_USART2 VET6_USART2
#define VGT6_USART3 VET6_USART3
#define VGT6_UART4 VET6_UART4
#define VGT6_UART5 VET6_UART5
#define VGT6_USART6 VET6_USART6
#define VGT6_USART_MAX VET6_USART_MAX

/* 函数别名 */
#define vgt6_usart_clk_enable vet6_usart_clk_enable
#define vgt6_usart_clk_disable vet6_usart_clk_disable
#define vgt6_usart_init vet6_usart_init
#define vgt6_usart_init_simple vet6_usart_init_simple
#define vgt6_usart_deinit vet6_usart_deinit
#define vgt6_usart_enable vet6_usart_enable
#define vgt6_usart_disable vet6_usart_disable
#define vgt6_usart_send_byte vet6_usart_send_byte
#define vgt6_usart_receive_byte vet6_usart_receive_byte
#define vgt6_usart_send vet6_usart_send
#define vgt6_usart_receive vet6_usart_receive
#define vgt6_usart_printf vet6_usart_printf
#define vgt6_usart_vprintf vet6_usart_vprintf

#endif /* __VGT6_USART_H */
