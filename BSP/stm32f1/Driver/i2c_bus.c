/**
 * @file i2c_bus.c
 * @brief STM32F1 软件I2C总线驱动
 * @note 使用 f103_gpio 接口实现软件I2C
 */

#include "driver.h"
#include "i2c/df_iic.h"
#include "f103_gpio.h"

/* ========== I2C1 配置: PB8=SCL, PB9=SDA ========== */
#define I2C1_SCL_PORT F103_GPIOB
#define I2C1_SCL_PIN F103_PIN_8
#define I2C1_SDA_PORT F103_GPIOB
#define I2C1_SDA_PIN F103_PIN_9

/**
 * @brief I2C1引脚初始化
 */
void iic1_pins_config(void)
{
    /* 配置 PB8(SCL), PB9(SDA) 为开漏输出50MHz（软件I2C需要模拟开漏特性） */
    f103_gpio_init_quick(I2C1_SCL_PORT, I2C1_SCL_PIN,
                         F103_GPIO_MODE_OUT_OD, F103_GPIO_SPEED_50MHZ);
    f103_gpio_init_quick(I2C1_SDA_PORT, I2C1_SDA_PIN,
                         F103_GPIO_MODE_OUT_OD, F103_GPIO_SPEED_50MHZ);

    /* 默认拉高 */
    f103_gpio_set(I2C1_SCL_PORT, I2C1_SCL_PIN);
    f103_gpio_set(I2C1_SDA_PORT, I2C1_SDA_PIN);
}

/**
 * @brief I2C1 SCL线控制
 */
void iic1_scl(uint8_t state)
{
    if (state)
    {
        f103_gpio_set(I2C1_SCL_PORT, I2C1_SCL_PIN);
    }
    else
    {
        f103_gpio_reset(I2C1_SCL_PORT, I2C1_SCL_PIN);
    }
}

/**
 * @brief I2C1 SDA线控制
 */
void iic1_sda(uint8_t state)
{
    if (state)
    {
        f103_gpio_set(I2C1_SDA_PORT, I2C1_SDA_PIN);
    }
    else
    {
        f103_gpio_reset(I2C1_SDA_PORT, I2C1_SDA_PIN);
    }
}

/**
 * @brief I2C1 SDA线设置为输入模式
 */
void iic1_sda_in(void)
{
    /* 重新配置SDA引脚为上拉输入 */
    f103_gpio_init_quick(I2C1_SDA_PORT, I2C1_SDA_PIN,
                         F103_GPIO_MODE_IPU, F103_GPIO_SPEED_2MHZ);
}

/**
 * @brief I2C1 SDA线设置为输出模式
 */
void iic1_sda_out(void)
{
    /* 重新配置SDA引脚为开漏输出 */
    f103_gpio_init_quick(I2C1_SDA_PORT, I2C1_SDA_PIN,
                         F103_GPIO_MODE_OUT_OD, F103_GPIO_SPEED_50MHZ);
}

/**
 * @brief I2C1 读取SDA线状态
 */
uint8_t iic1_read_sda(void)
{
    return f103_gpio_read(I2C1_SDA_PORT, I2C1_SDA_PIN);
}

/* 声明延时函数（来自 delay.c） */
extern void delay_us(uint32_t us);
extern void delay_ms(uint32_t ms);

/**
 * @brief I2C1软件IIC底层接口实例
 */
df_soft_iic_t i2c1_soft = {
    .init_flag = false,
    .gpio_init = iic1_pins_config,
    .delay_us = delay_us,
    .delay_ms = delay_ms,
    .scl = iic1_scl,
    .sda = iic1_sda,
    .sda_in = iic1_sda_in,
    .sda_out = iic1_sda_out,
    .read_sda = iic1_read_sda,
};

/**
 * @brief I2C1总线实例（统一接口）
 */
df_iic_t i2c1_bus = {
    .init_flag = false,
    .num = 1,
    .name = "I2C1",
    .init = NULL,
    .deinit = NULL,
    .write = NULL,
    .read = NULL,
    .check = NULL,
    .soft_iic = &i2c1_soft,
};