#include "driver.h"
#include "i2c/df_iic.h"
#include <driver.h>

/**
 * STM32F4 软件I2C总线驱动
 * 默认引脚: PB8(SCL), PB9(SDA)
 */

void iic1_pins_config(void)
{
    // 使能GPIOB时钟
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    // 配置 PB8, PB9 为开漏输出
    // PB8 配置
    GPIOB->MODER &= ~(0x3 << (8 * 2));
    GPIOB->MODER |= (0x1 << (8 * 2));   // 通用输出模式
    GPIOB->OTYPER |= (1 << 8);          // 开漏输出
    GPIOB->OSPEEDR |= (0x3 << (8 * 2)); // 高速
    GPIOB->PUPDR &= ~(0x3 << (8 * 2));
    GPIOB->PUPDR |= (0x1 << (8 * 2)); // 上拉

    // PB9 配置
    GPIOB->MODER &= ~(0x3 << (9 * 2));
    GPIOB->MODER |= (0x1 << (9 * 2));   // 通用输出模式
    GPIOB->OTYPER |= (1 << 9);          // 开漏输出
    GPIOB->OSPEEDR |= (0x3 << (9 * 2)); // 高速
    GPIOB->PUPDR &= ~(0x3 << (9 * 2));
    GPIOB->PUPDR |= (0x1 << (9 * 2)); // 上拉

    // 默认拉高
    GPIOB->ODR |= (1 << 8) | (1 << 9);
}

void iic1_scl(uint8_t state)
{
    if (state)
    {
        GPIOB->BSRRL = (1 << 8);
    }
    else
    {
        GPIOB->BSRRH = (1 << 8); // 使用BSRR高16位复位
    }
}

void iic1_sda(uint8_t state)
{
    if (state)
    {
        GPIOB->BSRRL = (1 << 9);
    }
    else
    {
        GPIOB->BSRRH = (1 << 9);
    }
}

void iic1_sda_in(void)
{
    // 将 PB9 配置为输入模式
    GPIOB->MODER &= ~(0x3 << (9 * 2)); // 输入模式
    GPIOB->PUPDR &= ~(0x3 << (9 * 2));
    GPIOB->PUPDR |= (0x1 << (9 * 2)); // 上拉
}

void iic1_sda_out(void)
{
    // 将 PB9 配置为开漏输出
    GPIOB->MODER &= ~(0x3 << (9 * 2));
    GPIOB->MODER |= (0x1 << (9 * 2)); // 通用输出模式
    GPIOB->OTYPER |= (1 << 9);        // 开漏输出
}

uint8_t iic1_read_sda(void)
{
    return (GPIOB->IDR & (1 << 9)) ? 1 : 0;
}

SIAS i2c1_bus = {
    .Soft_IIC_GPIO_Port_Init = iic1_pins_config,
    .delay_us = delay_us,
    .dealy_ms = delay_ms,
    .Soft_IIC_SCL = iic1_scl,
    .Soft_IIC_SDA = iic1_sda,
    .Soft_SDA_IN = iic1_sda_in,
    .Soft_SDA_OUT = iic1_sda_out,
    .Soft_READ_SDA = iic1_read_sda,
};
