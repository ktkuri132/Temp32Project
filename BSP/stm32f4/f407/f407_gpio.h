/**
 * @file f407_gpio.h
 * @brief STM32F407系列GPIO公共驱动头文件
 * @details 适用于所有STM32F407封装 (VET6/VGT6/ZET6/ZGT6等)
 *          F407拥有GPIOA-GPIOE共5组GPIO，每组16个引脚 (LQFP100封装)
 */

#ifndef __F407_GPIO_H
#define __F407_GPIO_H

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
        F407_GPIOA = 0,
        F407_GPIOB,
        F407_GPIOC,
        F407_GPIOD,
        F407_GPIOE,
        F407_GPIO_PORT_MAX
    } f407_gpio_port_t;

    /**
     * @brief GPIO引脚号
     */
    typedef enum
    {
        F407_PIN_0 = 0,
        F407_PIN_1,
        F407_PIN_2,
        F407_PIN_3,
        F407_PIN_4,
        F407_PIN_5,
        F407_PIN_6,
        F407_PIN_7,
        F407_PIN_8,
        F407_PIN_9,
        F407_PIN_10,
        F407_PIN_11,
        F407_PIN_12,
        F407_PIN_13,
        F407_PIN_14,
        F407_PIN_15,
        F407_PIN_MAX
    } f407_gpio_pin_t;

    /**
     * @brief GPIO模式
     */
    typedef enum
    {
        F407_GPIO_MODE_INPUT = 0x00,  /**< 输入模式 */
        F407_GPIO_MODE_OUTPUT = 0x01, /**< 通用输出模式 */
        F407_GPIO_MODE_AF = 0x02,     /**< 复用功能模式 */
        F407_GPIO_MODE_ANALOG = 0x03  /**< 模拟模式 */
    } f407_gpio_mode_t;

    /**
     * @brief GPIO输出类型
     */
    typedef enum
    {
        F407_GPIO_OTYPE_PP = 0, /**< 推挽输出 */
        F407_GPIO_OTYPE_OD = 1  /**< 开漏输出 */
    } f407_gpio_otype_t;

    /**
     * @brief GPIO输出速度
     */
    typedef enum
    {
        F407_GPIO_SPEED_LOW = 0x00,    /**< 2MHz */
        F407_GPIO_SPEED_MEDIUM = 0x01, /**< 25MHz */
        F407_GPIO_SPEED_FAST = 0x02,   /**< 50MHz */
        F407_GPIO_SPEED_HIGH = 0x03    /**< 100MHz */
    } f407_gpio_speed_t;

    /**
     * @brief GPIO上下拉配置
     */
    typedef enum
    {
        F407_GPIO_PUPD_NONE = 0x00, /**< 无上拉/下拉 */
        F407_GPIO_PUPD_UP = 0x01,   /**< 上拉 */
        F407_GPIO_PUPD_DOWN = 0x02  /**< 下拉 */
    } f407_gpio_pupd_t;

    /**
     * @brief GPIO复用功能
     */
    typedef enum
    {
        F407_GPIO_AF0_SYSTEM = 0x00,   /**< SYSTEM (MCO, JTAG, SWD等) */
        F407_GPIO_AF1_TIM1_2 = 0x01,   /**< TIM1, TIM2 */
        F407_GPIO_AF2_TIM3_5 = 0x02,   /**< TIM3, TIM4, TIM5 */
        F407_GPIO_AF3_TIM8_11 = 0x03,  /**< TIM8, TIM9, TIM10, TIM11 */
        F407_GPIO_AF4_I2C1_3 = 0x04,   /**< I2C1, I2C2, I2C3 */
        F407_GPIO_AF5_SPI1_2 = 0x05,   /**< SPI1, SPI2 */
        F407_GPIO_AF6_SPI3 = 0x06,     /**< SPI3 */
        F407_GPIO_AF7_USART1_3 = 0x07, /**< USART1, USART2, USART3 */
        F407_GPIO_AF8_USART4_6 = 0x08, /**< UART4, UART5, USART6 */
        F407_GPIO_AF9_CAN_TIM = 0x09,  /**< CAN1, CAN2, TIM12, TIM13, TIM14 */
        F407_GPIO_AF10_OTG = 0x0A,     /**< OTG_FS, OTG_HS */
        F407_GPIO_AF11_ETH = 0x0B,     /**< ETH */
        F407_GPIO_AF12_FSMC = 0x0C,    /**< FSMC, SDIO, OTG_HS */
        F407_GPIO_AF13_DCMI = 0x0D,    /**< DCMI */
        F407_GPIO_AF14 = 0x0E,         /**< Reserved */
        F407_GPIO_AF15_EVENTOUT = 0x0F /**< EVENTOUT */
    } f407_gpio_af_t;

    /**
     * @brief GPIO配置结构体
     */
    typedef struct
    {
        f407_gpio_port_t port;   /**< GPIO端口 */
        f407_gpio_pin_t pin;     /**< GPIO引脚 */
        f407_gpio_mode_t mode;   /**< GPIO模式 */
        f407_gpio_otype_t otype; /**< 输出类型 */
        f407_gpio_speed_t speed; /**< 输出速度 */
        f407_gpio_pupd_t pupd;   /**< 上下拉 */
        f407_gpio_af_t af;       /**< 复用功能 (仅AF模式有效) */
    } f407_gpio_config_t;

    /**
     * @brief GPIO引脚完整标识
     */
    typedef struct
    {
        f407_gpio_port_t port;
        f407_gpio_pin_t pin;
    } f407_gpio_id_t;

/*===========================================================================*/
/*                              宏定义                                        */
/*===========================================================================*/

/* 快速创建GPIO ID */
#define F407_GPIO_ID(p, n) ((f407_gpio_id_t){.port = (p), .pin = (n)})

/* 引脚定义 - Port A */
#define F407_PA0 F407_GPIO_ID(F407_GPIOA, F407_PIN_0)
#define F407_PA1 F407_GPIO_ID(F407_GPIOA, F407_PIN_1)
#define F407_PA2 F407_GPIO_ID(F407_GPIOA, F407_PIN_2)
#define F407_PA3 F407_GPIO_ID(F407_GPIOA, F407_PIN_3)
#define F407_PA4 F407_GPIO_ID(F407_GPIOA, F407_PIN_4)
#define F407_PA5 F407_GPIO_ID(F407_GPIOA, F407_PIN_5)
#define F407_PA6 F407_GPIO_ID(F407_GPIOA, F407_PIN_6)
#define F407_PA7 F407_GPIO_ID(F407_GPIOA, F407_PIN_7)
#define F407_PA8 F407_GPIO_ID(F407_GPIOA, F407_PIN_8)
#define F407_PA9 F407_GPIO_ID(F407_GPIOA, F407_PIN_9)
#define F407_PA10 F407_GPIO_ID(F407_GPIOA, F407_PIN_10)
#define F407_PA11 F407_GPIO_ID(F407_GPIOA, F407_PIN_11)
#define F407_PA12 F407_GPIO_ID(F407_GPIOA, F407_PIN_12)
#define F407_PA13 F407_GPIO_ID(F407_GPIOA, F407_PIN_13)
#define F407_PA14 F407_GPIO_ID(F407_GPIOA, F407_PIN_14)
#define F407_PA15 F407_GPIO_ID(F407_GPIOA, F407_PIN_15)

/* 引脚定义 - Port B */
#define F407_PB0 F407_GPIO_ID(F407_GPIOB, F407_PIN_0)
#define F407_PB1 F407_GPIO_ID(F407_GPIOB, F407_PIN_1)
#define F407_PB2 F407_GPIO_ID(F407_GPIOB, F407_PIN_2)
#define F407_PB3 F407_GPIO_ID(F407_GPIOB, F407_PIN_3)
#define F407_PB4 F407_GPIO_ID(F407_GPIOB, F407_PIN_4)
#define F407_PB5 F407_GPIO_ID(F407_GPIOB, F407_PIN_5)
#define F407_PB6 F407_GPIO_ID(F407_GPIOB, F407_PIN_6)
#define F407_PB7 F407_GPIO_ID(F407_GPIOB, F407_PIN_7)
#define F407_PB8 F407_GPIO_ID(F407_GPIOB, F407_PIN_8)
#define F407_PB9 F407_GPIO_ID(F407_GPIOB, F407_PIN_9)
#define F407_PB10 F407_GPIO_ID(F407_GPIOB, F407_PIN_10)
#define F407_PB11 F407_GPIO_ID(F407_GPIOB, F407_PIN_11)
#define F407_PB12 F407_GPIO_ID(F407_GPIOB, F407_PIN_12)
#define F407_PB13 F407_GPIO_ID(F407_GPIOB, F407_PIN_13)
#define F407_PB14 F407_GPIO_ID(F407_GPIOB, F407_PIN_14)
#define F407_PB15 F407_GPIO_ID(F407_GPIOB, F407_PIN_15)

/* 引脚定义 - Port C */
#define F407_PC0 F407_GPIO_ID(F407_GPIOC, F407_PIN_0)
#define F407_PC1 F407_GPIO_ID(F407_GPIOC, F407_PIN_1)
#define F407_PC2 F407_GPIO_ID(F407_GPIOC, F407_PIN_2)
#define F407_PC3 F407_GPIO_ID(F407_GPIOC, F407_PIN_3)
#define F407_PC4 F407_GPIO_ID(F407_GPIOC, F407_PIN_4)
#define F407_PC5 F407_GPIO_ID(F407_GPIOC, F407_PIN_5)
#define F407_PC6 F407_GPIO_ID(F407_GPIOC, F407_PIN_6)
#define F407_PC7 F407_GPIO_ID(F407_GPIOC, F407_PIN_7)
#define F407_PC8 F407_GPIO_ID(F407_GPIOC, F407_PIN_8)
#define F407_PC9 F407_GPIO_ID(F407_GPIOC, F407_PIN_9)
#define F407_PC10 F407_GPIO_ID(F407_GPIOC, F407_PIN_10)
#define F407_PC11 F407_GPIO_ID(F407_GPIOC, F407_PIN_11)
#define F407_PC12 F407_GPIO_ID(F407_GPIOC, F407_PIN_12)
#define F407_PC13 F407_GPIO_ID(F407_GPIOC, F407_PIN_13)
#define F407_PC14 F407_GPIO_ID(F407_GPIOC, F407_PIN_14)
#define F407_PC15 F407_GPIO_ID(F407_GPIOC, F407_PIN_15)

/* 引脚定义 - Port D */
#define F407_PD0 F407_GPIO_ID(F407_GPIOD, F407_PIN_0)
#define F407_PD1 F407_GPIO_ID(F407_GPIOD, F407_PIN_1)
#define F407_PD2 F407_GPIO_ID(F407_GPIOD, F407_PIN_2)
#define F407_PD3 F407_GPIO_ID(F407_GPIOD, F407_PIN_3)
#define F407_PD4 F407_GPIO_ID(F407_GPIOD, F407_PIN_4)
#define F407_PD5 F407_GPIO_ID(F407_GPIOD, F407_PIN_5)
#define F407_PD6 F407_GPIO_ID(F407_GPIOD, F407_PIN_6)
#define F407_PD7 F407_GPIO_ID(F407_GPIOD, F407_PIN_7)
#define F407_PD8 F407_GPIO_ID(F407_GPIOD, F407_PIN_8)
#define F407_PD9 F407_GPIO_ID(F407_GPIOD, F407_PIN_9)
#define F407_PD10 F407_GPIO_ID(F407_GPIOD, F407_PIN_10)
#define F407_PD11 F407_GPIO_ID(F407_GPIOD, F407_PIN_11)
#define F407_PD12 F407_GPIO_ID(F407_GPIOD, F407_PIN_12)
#define F407_PD13 F407_GPIO_ID(F407_GPIOD, F407_PIN_13)
#define F407_PD14 F407_GPIO_ID(F407_GPIOD, F407_PIN_14)
#define F407_PD15 F407_GPIO_ID(F407_GPIOD, F407_PIN_15)

/* 引脚定义 - Port E */
#define F407_PE0 F407_GPIO_ID(F407_GPIOE, F407_PIN_0)
#define F407_PE1 F407_GPIO_ID(F407_GPIOE, F407_PIN_1)
#define F407_PE2 F407_GPIO_ID(F407_GPIOE, F407_PIN_2)
#define F407_PE3 F407_GPIO_ID(F407_GPIOE, F407_PIN_3)
#define F407_PE4 F407_GPIO_ID(F407_GPIOE, F407_PIN_4)
#define F407_PE5 F407_GPIO_ID(F407_GPIOE, F407_PIN_5)
#define F407_PE6 F407_GPIO_ID(F407_GPIOE, F407_PIN_6)
#define F407_PE7 F407_GPIO_ID(F407_GPIOE, F407_PIN_7)
#define F407_PE8 F407_GPIO_ID(F407_GPIOE, F407_PIN_8)
#define F407_PE9 F407_GPIO_ID(F407_GPIOE, F407_PIN_9)
#define F407_PE10 F407_GPIO_ID(F407_GPIOE, F407_PIN_10)
#define F407_PE11 F407_GPIO_ID(F407_GPIOE, F407_PIN_11)
#define F407_PE12 F407_GPIO_ID(F407_GPIOE, F407_PIN_12)
#define F407_PE13 F407_GPIO_ID(F407_GPIOE, F407_PIN_13)
#define F407_PE14 F407_GPIO_ID(F407_GPIOE, F407_PIN_14)
#define F407_PE15 F407_GPIO_ID(F407_GPIOE, F407_PIN_15)

    /*===========================================================================*/
    /*                              API函数声明                                   */
    /*===========================================================================*/

    void f407_gpio_clk_enable(f407_gpio_port_t port);
    void f407_gpio_clk_enable_all(void);
    int f407_gpio_config(const f407_gpio_config_t *config);
    int f407_gpio_config_output_pp(f407_gpio_id_t id);
    int f407_gpio_config_output_od(f407_gpio_id_t id);
    int f407_gpio_config_input_float(f407_gpio_id_t id);
    int f407_gpio_config_input_pu(f407_gpio_id_t id);
    int f407_gpio_config_input_pd(f407_gpio_id_t id);
    int f407_gpio_config_analog(f407_gpio_id_t id);
    int f407_gpio_config_af(f407_gpio_id_t id, f407_gpio_af_t af);
    int f407_gpio_config_af_otype(f407_gpio_id_t id, f407_gpio_af_t af, f407_gpio_otype_t otype);
    void f407_gpio_set(f407_gpio_id_t id);
    void f407_gpio_reset(f407_gpio_id_t id);
    void f407_gpio_toggle(f407_gpio_id_t id);
    void f407_gpio_write(f407_gpio_id_t id, uint8_t state);
    uint8_t f407_gpio_read(f407_gpio_id_t id);
    uint8_t f407_gpio_read_output(f407_gpio_id_t id);
    void f407_gpio_write_port(f407_gpio_port_t port, uint16_t value);
    uint16_t f407_gpio_read_port(f407_gpio_port_t port);
    int f407_gpio_lock(f407_gpio_id_t id);
    GPIO_TypeDef *f407_gpio_get_base(f407_gpio_port_t port);

#ifdef __cplusplus
}
#endif

#endif /* __F407_GPIO_H */
