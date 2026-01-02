/**
 * @file f103_gpio.h
 * @brief STM32F103系列GPIO公共驱动头文件
 * @details 适用于所有STM32F103封装 (C8T6/C6T6/RBT6/RCT6/VET6等)
 *          F103拥有GPIOA-GPIOE共5组GPIO，每组16个引脚
 */

#ifndef __F103_GPIO_H
#define __F103_GPIO_H

#include <stm32f10x.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /*===========================================================================*/
    /*                              类型定义                                      */
    /*===========================================================================*/

    /**
     * @brief GPIO端口枚举
     */
    typedef enum
    {
        F103_GPIOA = 0,
        F103_GPIOB,
        F103_GPIOC,
        F103_GPIOD,
        F103_GPIOE,
        F103_GPIO_PORT_MAX
    } f103_gpio_port_t;

    /**
     * @brief GPIO引脚号
     */
    typedef enum
    {
        F103_PIN_0 = 0,
        F103_PIN_1,
        F103_PIN_2,
        F103_PIN_3,
        F103_PIN_4,
        F103_PIN_5,
        F103_PIN_6,
        F103_PIN_7,
        F103_PIN_8,
        F103_PIN_9,
        F103_PIN_10,
        F103_PIN_11,
        F103_PIN_12,
        F103_PIN_13,
        F103_PIN_14,
        F103_PIN_15,
        F103_PIN_MAX
    } f103_gpio_pin_t;

    /**
     * @brief GPIO模式 (STM32F1特有的配置方式)
     */
    typedef enum
    {
        F103_GPIO_MODE_AIN = 0x00,         /**< 模拟输入 */
        F103_GPIO_MODE_IN_FLOATING = 0x04, /**< 浮空输入 */
        F103_GPIO_MODE_IPD = 0x28,         /**< 下拉输入 */
        F103_GPIO_MODE_IPU = 0x48,         /**< 上拉输入 */
        F103_GPIO_MODE_OUT_OD = 0x14,      /**< 开漏输出 */
        F103_GPIO_MODE_OUT_PP = 0x10,      /**< 推挽输出 */
        F103_GPIO_MODE_AF_OD = 0x1C,       /**< 复用开漏输出 */
        F103_GPIO_MODE_AF_PP = 0x18        /**< 复用推挽输出 */
    } f103_gpio_mode_t;

    /**
     * @brief GPIO输出速度
     */
    typedef enum
    {
        F103_GPIO_SPEED_2MHZ = 0x02,  /**< 2MHz */
        F103_GPIO_SPEED_10MHZ = 0x01, /**< 10MHz */
        F103_GPIO_SPEED_50MHZ = 0x03  /**< 50MHz */
    } f103_gpio_speed_t;

    /**
     * @brief GPIO配置结构体
     */
    typedef struct
    {
        f103_gpio_port_t port;   /**< GPIO端口 */
        f103_gpio_pin_t pin;     /**< GPIO引脚 */
        f103_gpio_mode_t mode;   /**< GPIO模式 */
        f103_gpio_speed_t speed; /**< 输出速度 (仅输出模式有效) */
    } f103_gpio_config_t;

    /*===========================================================================*/
    /*                              引脚快捷定义                                  */
    /*===========================================================================*/

#define F103_PA0 F103_GPIOA, F103_PIN_0
#define F103_PA1 F103_GPIOA, F103_PIN_1
#define F103_PA2 F103_GPIOA, F103_PIN_2
#define F103_PA3 F103_GPIOA, F103_PIN_3
#define F103_PA4 F103_GPIOA, F103_PIN_4
#define F103_PA5 F103_GPIOA, F103_PIN_5
#define F103_PA6 F103_GPIOA, F103_PIN_6
#define F103_PA7 F103_GPIOA, F103_PIN_7
#define F103_PA8 F103_GPIOA, F103_PIN_8
#define F103_PA9 F103_GPIOA, F103_PIN_9
#define F103_PA10 F103_GPIOA, F103_PIN_10
#define F103_PA11 F103_GPIOA, F103_PIN_11
#define F103_PA12 F103_GPIOA, F103_PIN_12
#define F103_PA13 F103_GPIOA, F103_PIN_13
#define F103_PA14 F103_GPIOA, F103_PIN_14
#define F103_PA15 F103_GPIOA, F103_PIN_15

#define F103_PB0 F103_GPIOB, F103_PIN_0
#define F103_PB1 F103_GPIOB, F103_PIN_1
#define F103_PB2 F103_GPIOB, F103_PIN_2
#define F103_PB3 F103_GPIOB, F103_PIN_3
#define F103_PB4 F103_GPIOB, F103_PIN_4
#define F103_PB5 F103_GPIOB, F103_PIN_5
#define F103_PB6 F103_GPIOB, F103_PIN_6
#define F103_PB7 F103_GPIOB, F103_PIN_7
#define F103_PB8 F103_GPIOB, F103_PIN_8
#define F103_PB9 F103_GPIOB, F103_PIN_9
#define F103_PB10 F103_GPIOB, F103_PIN_10
#define F103_PB11 F103_GPIOB, F103_PIN_11
#define F103_PB12 F103_GPIOB, F103_PIN_12
#define F103_PB13 F103_GPIOB, F103_PIN_13
#define F103_PB14 F103_GPIOB, F103_PIN_14
#define F103_PB15 F103_GPIOB, F103_PIN_15

#define F103_PC13 F103_GPIOC, F103_PIN_13
#define F103_PC14 F103_GPIOC, F103_PIN_14
#define F103_PC15 F103_GPIOC, F103_PIN_15

    /*===========================================================================*/
    /*                              API函数声明                                   */
    /*===========================================================================*/

    /**
     * @brief 使能GPIO端口时钟
     * @param port GPIO端口
     */
    void f103_gpio_clk_enable(f103_gpio_port_t port);

    /**
     * @brief 禁用GPIO端口时钟
     * @param port GPIO端口
     */
    void f103_gpio_clk_disable(f103_gpio_port_t port);

    /**
     * @brief 初始化GPIO引脚
     * @param config 配置结构体指针
     * @return 0成功，-1失败
     */
    int f103_gpio_init(const f103_gpio_config_t *config);

    /**
     * @brief 快速初始化GPIO引脚
     * @param port GPIO端口
     * @param pin 引脚号
     * @param mode 模式
     * @param speed 速度
     * @return 0成功，-1失败
     */
    int f103_gpio_init_quick(f103_gpio_port_t port, f103_gpio_pin_t pin,
                             f103_gpio_mode_t mode, f103_gpio_speed_t speed);

    /**
     * @brief 设置GPIO输出电平
     * @param port GPIO端口
     * @param pin 引脚号
     * @param level 电平 (0低电平，非0高电平)
     */
    void f103_gpio_write(f103_gpio_port_t port, f103_gpio_pin_t pin, uint8_t level);

    /**
     * @brief 读取GPIO输入电平
     * @param port GPIO端口
     * @param pin 引脚号
     * @return 引脚电平 (0或1)
     */
    uint8_t f103_gpio_read(f103_gpio_port_t port, f103_gpio_pin_t pin);

    /**
     * @brief 翻转GPIO输出电平
     * @param port GPIO端口
     * @param pin 引脚号
     */
    void f103_gpio_toggle(f103_gpio_port_t port, f103_gpio_pin_t pin);

    /**
     * @brief 设置GPIO高电平
     * @param port GPIO端口
     * @param pin 引脚号
     */
    void f103_gpio_set(f103_gpio_port_t port, f103_gpio_pin_t pin);

    /**
     * @brief 设置GPIO低电平
     * @param port GPIO端口
     * @param pin 引脚号
     */
    void f103_gpio_reset(f103_gpio_port_t port, f103_gpio_pin_t pin);

#ifdef __cplusplus
}
#endif

#endif /* __F103_GPIO_H */
