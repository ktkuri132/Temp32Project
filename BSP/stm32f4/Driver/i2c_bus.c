/**
 * @file i2c_bus.c
 * @brief STM32F4 软件 I2C 总线驱动
 * @note 使用 f407_gpio 接口实现软件 I2C
 */

#include "driver.h"
#include "i2c/df_iic.h"
#include "f407_gpio.h"

/*============================ I2C1 配置 ============================*/
/* PB8=SCL, PB9=SDA */
#define I2C1_SCL_ID F407_GPIO_ID(F407_GPIOB, F407_PIN_8)
#define I2C1_SDA_ID F407_GPIO_ID(F407_GPIOB, F407_PIN_9)

/*============================ 底层接口实现 ============================*/

/**
 * @brief I2C1 引脚初始化
 */
void iic1_pins_config(void)
{
    /* 配置 PB8(SCL), PB9(SDA) 为开漏输出 */
    f407_gpio_config_output_od(I2C1_SCL_ID);
    f407_gpio_config_output_od(I2C1_SDA_ID);

    /* 默认拉高 */
    f407_gpio_set(I2C1_SCL_ID);
    f407_gpio_set(I2C1_SDA_ID);
}

/**
 * @brief I2C1 SCL 线控制
 */
void iic1_scl(uint8_t state)
{
    if (state)
    {
        f407_gpio_set(I2C1_SCL_ID);
    }
    else
    {
        f407_gpio_reset(I2C1_SCL_ID);
    }
}

/**
 * @brief I2C1 SDA 线控制
 */
void iic1_sda(uint8_t state)
{
    if (state)
    {
        f407_gpio_set(I2C1_SDA_ID);
    }
    else
    {
        f407_gpio_reset(I2C1_SDA_ID);
    }
}

/**
 * @brief I2C1 SDA 线设置为输入模式
 */
void iic1_sda_in(void)
{
    /* 配置 SDA 引脚为上拉输入 */
    f407_gpio_config_input_pu(I2C1_SDA_ID);
}

/**
 * @brief I2C1 SDA 线设置为输出模式
 */
void iic1_sda_out(void)
{
    /* 配置 SDA 引脚为开漏输出 */
    f407_gpio_config_output_od(I2C1_SDA_ID);
}

/**
 * @brief I2C1 读取 SDA 线状态
 */
uint8_t iic1_read_sda(void)
{
    return f407_gpio_read(I2C1_SDA_ID);
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
