/**
 * @file vet6_gpio.h
 * @brief STM32F407VET6 GPIO板级驱动头文件
 * @details 提供所有GPIO端口的初始化和操作API
 *          VET6拥有GPIOA-GPIOE共5组GPIO，每组16个引脚
 */

#ifndef __VET6_GPIO_H
#define __VET6_GPIO_H

#include <stm32f4xx.h>
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
        VET6_GPIOA = 0,
        VET6_GPIOB,
        VET6_GPIOC,
        VET6_GPIOD,
        VET6_GPIOE,
        VET6_GPIO_PORT_MAX
    } vet6_gpio_port_t;

    /**
     * @brief GPIO引脚号
     */
    typedef enum
    {
        VET6_PIN_0 = 0,
        VET6_PIN_1,
        VET6_PIN_2,
        VET6_PIN_3,
        VET6_PIN_4,
        VET6_PIN_5,
        VET6_PIN_6,
        VET6_PIN_7,
        VET6_PIN_8,
        VET6_PIN_9,
        VET6_PIN_10,
        VET6_PIN_11,
        VET6_PIN_12,
        VET6_PIN_13,
        VET6_PIN_14,
        VET6_PIN_15,
        VET6_PIN_MAX
    } vet6_gpio_pin_t;

    /**
     * @brief GPIO模式
     */
    typedef enum
    {
        VET6_GPIO_MODE_INPUT = 0x00,  // 输入模式
        VET6_GPIO_MODE_OUTPUT = 0x01, // 通用输出模式
        VET6_GPIO_MODE_AF = 0x02,     // 复用功能模式
        VET6_GPIO_MODE_ANALOG = 0x03  // 模拟模式
    } vet6_gpio_mode_t;

    /**
     * @brief GPIO输出类型
     */
    typedef enum
    {
        VET6_GPIO_OTYPE_PP = 0, // 推挽输出
        VET6_GPIO_OTYPE_OD = 1  // 开漏输出
    } vet6_gpio_otype_t;

    /**
     * @brief GPIO输出速度
     */
    typedef enum
    {
        VET6_GPIO_SPEED_LOW = 0x00,    // 2MHz
        VET6_GPIO_SPEED_MEDIUM = 0x01, // 25MHz
        VET6_GPIO_SPEED_FAST = 0x02,   // 50MHz
        VET6_GPIO_SPEED_HIGH = 0x03    // 100MHz
    } vet6_gpio_speed_t;

    /**
     * @brief GPIO上下拉配置
     */
    typedef enum
    {
        VET6_GPIO_PUPD_NONE = 0x00, // 无上拉/下拉
        VET6_GPIO_PUPD_UP = 0x01,   // 上拉
        VET6_GPIO_PUPD_DOWN = 0x02  // 下拉
    } vet6_gpio_pupd_t;

    /**
     * @brief GPIO复用功能
     */
    typedef enum
    {
        VET6_GPIO_AF0_SYSTEM = 0x00,   // SYSTEM (MCO, JTAG, SWD等)
        VET6_GPIO_AF1_TIM1_2 = 0x01,   // TIM1, TIM2
        VET6_GPIO_AF2_TIM3_5 = 0x02,   // TIM3, TIM4, TIM5
        VET6_GPIO_AF3_TIM8_11 = 0x03,  // TIM8, TIM9, TIM10, TIM11
        VET6_GPIO_AF4_I2C1_3 = 0x04,   // I2C1, I2C2, I2C3
        VET6_GPIO_AF5_SPI1_2 = 0x05,   // SPI1, SPI2
        VET6_GPIO_AF6_SPI3 = 0x06,     // SPI3
        VET6_GPIO_AF7_USART1_3 = 0x07, // USART1, USART2, USART3
        VET6_GPIO_AF8_USART4_6 = 0x08, // UART4, UART5, USART6
        VET6_GPIO_AF9_CAN_TIM = 0x09,  // CAN1, CAN2, TIM12, TIM13, TIM14
        VET6_GPIO_AF10_OTG = 0x0A,     // OTG_FS, OTG_HS
        VET6_GPIO_AF11_ETH = 0x0B,     // ETH
        VET6_GPIO_AF12_FSMC = 0x0C,    // FSMC, SDIO, OTG_HS
        VET6_GPIO_AF13_DCMI = 0x0D,    // DCMI
        VET6_GPIO_AF14 = 0x0E,         // Reserved
        VET6_GPIO_AF15_EVENTOUT = 0x0F // EVENTOUT
    } vet6_gpio_af_t;

    /**
     * @brief GPIO配置结构体
     */
    typedef struct
    {
        vet6_gpio_port_t port;   // GPIO端口
        vet6_gpio_pin_t pin;     // GPIO引脚
        vet6_gpio_mode_t mode;   // GPIO模式
        vet6_gpio_otype_t otype; // 输出类型
        vet6_gpio_speed_t speed; // 输出速度
        vet6_gpio_pupd_t pupd;   // 上下拉
        vet6_gpio_af_t af;       // 复用功能 (仅AF模式有效)
    } vet6_gpio_config_t;

    /**
     * @brief GPIO引脚完整标识
     */
    typedef struct
    {
        vet6_gpio_port_t port;
        vet6_gpio_pin_t pin;
    } vet6_gpio_id_t;

/*===========================================================================*/
/*                              宏定义                                        */
/*===========================================================================*/

/* 快速创建GPIO ID */
#define VET6_GPIO_ID(p, n) ((vet6_gpio_id_t){.port = (p), .pin = (n)})

/* 常用引脚定义 */
#define VET6_PA0 VET6_GPIO_ID(VET6_GPIOA, VET6_PIN_0)
#define VET6_PA1 VET6_GPIO_ID(VET6_GPIOA, VET6_PIN_1)
#define VET6_PA2 VET6_GPIO_ID(VET6_GPIOA, VET6_PIN_2)
#define VET6_PA3 VET6_GPIO_ID(VET6_GPIOA, VET6_PIN_3)
#define VET6_PA4 VET6_GPIO_ID(VET6_GPIOA, VET6_PIN_4)
#define VET6_PA5 VET6_GPIO_ID(VET6_GPIOA, VET6_PIN_5)
#define VET6_PA6 VET6_GPIO_ID(VET6_GPIOA, VET6_PIN_6)
#define VET6_PA7 VET6_GPIO_ID(VET6_GPIOA, VET6_PIN_7)
#define VET6_PA8 VET6_GPIO_ID(VET6_GPIOA, VET6_PIN_8)
#define VET6_PA9 VET6_GPIO_ID(VET6_GPIOA, VET6_PIN_9)
#define VET6_PA10 VET6_GPIO_ID(VET6_GPIOA, VET6_PIN_10)
#define VET6_PA11 VET6_GPIO_ID(VET6_GPIOA, VET6_PIN_11)
#define VET6_PA12 VET6_GPIO_ID(VET6_GPIOA, VET6_PIN_12)
#define VET6_PA13 VET6_GPIO_ID(VET6_GPIOA, VET6_PIN_13)
#define VET6_PA14 VET6_GPIO_ID(VET6_GPIOA, VET6_PIN_14)
#define VET6_PA15 VET6_GPIO_ID(VET6_GPIOA, VET6_PIN_15)

#define VET6_PB0 VET6_GPIO_ID(VET6_GPIOB, VET6_PIN_0)
#define VET6_PB1 VET6_GPIO_ID(VET6_GPIOB, VET6_PIN_1)
#define VET6_PB2 VET6_GPIO_ID(VET6_GPIOB, VET6_PIN_2)
#define VET6_PB3 VET6_GPIO_ID(VET6_GPIOB, VET6_PIN_3)
#define VET6_PB4 VET6_GPIO_ID(VET6_GPIOB, VET6_PIN_4)
#define VET6_PB5 VET6_GPIO_ID(VET6_GPIOB, VET6_PIN_5)
#define VET6_PB6 VET6_GPIO_ID(VET6_GPIOB, VET6_PIN_6)
#define VET6_PB7 VET6_GPIO_ID(VET6_GPIOB, VET6_PIN_7)
#define VET6_PB8 VET6_GPIO_ID(VET6_GPIOB, VET6_PIN_8)
#define VET6_PB9 VET6_GPIO_ID(VET6_GPIOB, VET6_PIN_9)
#define VET6_PB10 VET6_GPIO_ID(VET6_GPIOB, VET6_PIN_10)
#define VET6_PB11 VET6_GPIO_ID(VET6_GPIOB, VET6_PIN_11)
#define VET6_PB12 VET6_GPIO_ID(VET6_GPIOB, VET6_PIN_12)
#define VET6_PB13 VET6_GPIO_ID(VET6_GPIOB, VET6_PIN_13)
#define VET6_PB14 VET6_GPIO_ID(VET6_GPIOB, VET6_PIN_14)
#define VET6_PB15 VET6_GPIO_ID(VET6_GPIOB, VET6_PIN_15)

#define VET6_PC0 VET6_GPIO_ID(VET6_GPIOC, VET6_PIN_0)
#define VET6_PC1 VET6_GPIO_ID(VET6_GPIOC, VET6_PIN_1)
#define VET6_PC2 VET6_GPIO_ID(VET6_GPIOC, VET6_PIN_2)
#define VET6_PC3 VET6_GPIO_ID(VET6_GPIOC, VET6_PIN_3)
#define VET6_PC4 VET6_GPIO_ID(VET6_GPIOC, VET6_PIN_4)
#define VET6_PC5 VET6_GPIO_ID(VET6_GPIOC, VET6_PIN_5)
#define VET6_PC6 VET6_GPIO_ID(VET6_GPIOC, VET6_PIN_6)
#define VET6_PC7 VET6_GPIO_ID(VET6_GPIOC, VET6_PIN_7)
#define VET6_PC8 VET6_GPIO_ID(VET6_GPIOC, VET6_PIN_8)
#define VET6_PC9 VET6_GPIO_ID(VET6_GPIOC, VET6_PIN_9)
#define VET6_PC10 VET6_GPIO_ID(VET6_GPIOC, VET6_PIN_10)
#define VET6_PC11 VET6_GPIO_ID(VET6_GPIOC, VET6_PIN_11)
#define VET6_PC12 VET6_GPIO_ID(VET6_GPIOC, VET6_PIN_12)
#define VET6_PC13 VET6_GPIO_ID(VET6_GPIOC, VET6_PIN_13)
#define VET6_PC14 VET6_GPIO_ID(VET6_GPIOC, VET6_PIN_14)
#define VET6_PC15 VET6_GPIO_ID(VET6_GPIOC, VET6_PIN_15)

#define VET6_PD0 VET6_GPIO_ID(VET6_GPIOD, VET6_PIN_0)
#define VET6_PD1 VET6_GPIO_ID(VET6_GPIOD, VET6_PIN_1)
#define VET6_PD2 VET6_GPIO_ID(VET6_GPIOD, VET6_PIN_2)
#define VET6_PD3 VET6_GPIO_ID(VET6_GPIOD, VET6_PIN_3)
#define VET6_PD4 VET6_GPIO_ID(VET6_GPIOD, VET6_PIN_4)
#define VET6_PD5 VET6_GPIO_ID(VET6_GPIOD, VET6_PIN_5)
#define VET6_PD6 VET6_GPIO_ID(VET6_GPIOD, VET6_PIN_6)
#define VET6_PD7 VET6_GPIO_ID(VET6_GPIOD, VET6_PIN_7)
#define VET6_PD8 VET6_GPIO_ID(VET6_GPIOD, VET6_PIN_8)
#define VET6_PD9 VET6_GPIO_ID(VET6_GPIOD, VET6_PIN_9)
#define VET6_PD10 VET6_GPIO_ID(VET6_GPIOD, VET6_PIN_10)
#define VET6_PD11 VET6_GPIO_ID(VET6_GPIOD, VET6_PIN_11)
#define VET6_PD12 VET6_GPIO_ID(VET6_GPIOD, VET6_PIN_12)
#define VET6_PD13 VET6_GPIO_ID(VET6_GPIOD, VET6_PIN_13)
#define VET6_PD14 VET6_GPIO_ID(VET6_GPIOD, VET6_PIN_14)
#define VET6_PD15 VET6_GPIO_ID(VET6_GPIOD, VET6_PIN_15)

#define VET6_PE0 VET6_GPIO_ID(VET6_GPIOE, VET6_PIN_0)
#define VET6_PE1 VET6_GPIO_ID(VET6_GPIOE, VET6_PIN_1)
#define VET6_PE2 VET6_GPIO_ID(VET6_GPIOE, VET6_PIN_2)
#define VET6_PE3 VET6_GPIO_ID(VET6_GPIOE, VET6_PIN_3)
#define VET6_PE4 VET6_GPIO_ID(VET6_GPIOE, VET6_PIN_4)
#define VET6_PE5 VET6_GPIO_ID(VET6_GPIOE, VET6_PIN_5)
#define VET6_PE6 VET6_GPIO_ID(VET6_GPIOE, VET6_PIN_6)
#define VET6_PE7 VET6_GPIO_ID(VET6_GPIOE, VET6_PIN_7)
#define VET6_PE8 VET6_GPIO_ID(VET6_GPIOE, VET6_PIN_8)
#define VET6_PE9 VET6_GPIO_ID(VET6_GPIOE, VET6_PIN_9)
#define VET6_PE10 VET6_GPIO_ID(VET6_GPIOE, VET6_PIN_10)
#define VET6_PE11 VET6_GPIO_ID(VET6_GPIOE, VET6_PIN_11)
#define VET6_PE12 VET6_GPIO_ID(VET6_GPIOE, VET6_PIN_12)
#define VET6_PE13 VET6_GPIO_ID(VET6_GPIOE, VET6_PIN_13)
#define VET6_PE14 VET6_GPIO_ID(VET6_GPIOE, VET6_PIN_14)
#define VET6_PE15 VET6_GPIO_ID(VET6_GPIOE, VET6_PIN_15)

    /*===========================================================================*/
    /*                              API函数声明                                   */
    /*===========================================================================*/

    /**
     * @brief 使能GPIO端口时钟
     * @param port GPIO端口
     */
    void vet6_gpio_clk_enable(vet6_gpio_port_t port);

    /**
     * @brief 使能所有GPIO端口时钟
     */
    void vet6_gpio_clk_enable_all(void);

    /**
     * @brief 配置GPIO引脚
     * @param config 配置结构体指针
     * @return 0:成功, -1:失败
     */
    int vet6_gpio_config(const vet6_gpio_config_t *config);

    /**
     * @brief 快速配置GPIO引脚为推挽输出
     * @param id GPIO引脚标识
     * @return 0:成功, -1:失败
     */
    int vet6_gpio_config_output_pp(vet6_gpio_id_t id);

    /**
     * @brief 快速配置GPIO引脚为开漏输出
     * @param id GPIO引脚标识
     * @return 0:成功, -1:失败
     */
    int vet6_gpio_config_output_od(vet6_gpio_id_t id);

    /**
     * @brief 快速配置GPIO引脚为浮空输入
     * @param id GPIO引脚标识
     * @return 0:成功, -1:失败
     */
    int vet6_gpio_config_input_float(vet6_gpio_id_t id);

    /**
     * @brief 快速配置GPIO引脚为上拉输入
     * @param id GPIO引脚标识
     * @return 0:成功, -1:失败
     */
    int vet6_gpio_config_input_pu(vet6_gpio_id_t id);

    /**
     * @brief 快速配置GPIO引脚为下拉输入
     * @param id GPIO引脚标识
     * @return 0:成功, -1:失败
     */
    int vet6_gpio_config_input_pd(vet6_gpio_id_t id);

    /**
     * @brief 快速配置GPIO引脚为模拟模式
     * @param id GPIO引脚标识
     * @return 0:成功, -1:失败
     */
    int vet6_gpio_config_analog(vet6_gpio_id_t id);

    /**
     * @brief 配置GPIO引脚复用功能
     * @param id GPIO引脚标识
     * @param af 复用功能号
     * @return 0:成功, -1:失败
     */
    int vet6_gpio_config_af(vet6_gpio_id_t id, vet6_gpio_af_t af);

    /**
     * @brief 配置GPIO引脚复用功能(带输出类型)
     * @param id GPIO引脚标识
     * @param af 复用功能号
     * @param otype 输出类型
     * @return 0:成功, -1:失败
     */
    int vet6_gpio_config_af_otype(vet6_gpio_id_t id, vet6_gpio_af_t af, vet6_gpio_otype_t otype);

    /**
     * @brief 设置GPIO引脚输出高电平
     * @param id GPIO引脚标识
     */
    void vet6_gpio_set(vet6_gpio_id_t id);

    /**
     * @brief 设置GPIO引脚输出低电平
     * @param id GPIO引脚标识
     */
    void vet6_gpio_reset(vet6_gpio_id_t id);

    /**
     * @brief 翻转GPIO引脚输出电平
     * @param id GPIO引脚标识
     */
    void vet6_gpio_toggle(vet6_gpio_id_t id);

    /**
     * @brief 写GPIO引脚输出电平
     * @param id GPIO引脚标识
     * @param state 0:低电平, 非0:高电平
     */
    void vet6_gpio_write(vet6_gpio_id_t id, uint8_t state);

    /**
     * @brief 读取GPIO引脚输入电平
     * @param id GPIO引脚标识
     * @return 0:低电平, 1:高电平
     */
    uint8_t vet6_gpio_read(vet6_gpio_id_t id);

    /**
     * @brief 读取GPIO引脚输出电平
     * @param id GPIO引脚标识
     * @return 0:低电平, 1:高电平
     */
    uint8_t vet6_gpio_read_output(vet6_gpio_id_t id);

    /**
     * @brief 写GPIO端口整体输出
     * @param port GPIO端口
     * @param value 16位数据
     */
    void vet6_gpio_write_port(vet6_gpio_port_t port, uint16_t value);

    /**
     * @brief 读取GPIO端口整体输入
     * @param port GPIO端口
     * @return 16位数据
     */
    uint16_t vet6_gpio_read_port(vet6_gpio_port_t port);

    /**
     * @brief 锁定GPIO引脚配置
     * @param id GPIO引脚标识
     * @return 0:成功, -1:失败
     */
    int vet6_gpio_lock(vet6_gpio_id_t id);

    /**
     * @brief 获取GPIO外设基地址
     * @param port GPIO端口
     * @return GPIO外设指针
     */
    GPIO_TypeDef *vet6_gpio_get_base(vet6_gpio_port_t port);

#ifdef __cplusplus
}
#endif

#endif /* __VET6_GPIO_H */
