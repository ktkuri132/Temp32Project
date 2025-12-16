#include "stm32f10x.h"
//****************引脚配置*********************

#include "df_iic.h"
#include <driver.h>


void iic1_pins_config(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    // 配置 PB8, PB9 为开漏输出 50MHz，用于软件IIC
    // PB8 配置在 CRH 位[3:0]
    // MODE8 = 0b11 (50MHz), CNF8 = 0b01 (开漏通用输出)
    GPIOB->CRH &= ~(0xF << 0);
    GPIOB->CRH |= ((0x3 << 0) | (0x1 << 2));
    // PB9 配置在 CRH 位[7:4]
    // MODE9 = 0b11 (50MHz), CNF9 = 0b01 (开漏通用输出)
    GPIOB->CRH &= ~(0xF << 4);
    GPIOB->CRH |= ((0x3 << 4) | (0x1 << 6));
    // 默认拉高（外部需上拉，或使能内部上拉）
    // 使能上拉：将 ODR 对应位置置1，并配置为输入上拉时使用。
    GPIOB->ODR |= (1 << 8) | (1 << 9);
}

void iic1_scl(uint8_t state)
{
    if (state)
    {
        GPIOB->BSRR = (1 << 8);
    }
    else
    {
        GPIOB->BRR = (1 << 8);
    }
}

void iic1_sda(uint8_t state)
{
    if (state)
    {
        GPIOB->BSRR = (1 << 9);
    }
    else
    {
        GPIOB->BRR = (1 << 9);
    }
}

void iic1_sda_in(void)
{
    // 将 PB9 配置为输入上拉：MODE9=00, CNF9=10
    GPIOB->CRH &= ~(0xF << 4);
    GPIOB->CRH |= (0x2 << 6);
    GPIOB->ODR |= (1 << 9); // 上拉
}

void iic1_sda_out(void)
{
    // 将 PB9 配置为开漏输出 50MHz：MODE9=11, CNF9=01
    GPIOB->CRH &= ~(0xF << 4);
    GPIOB->CRH |= ((0x3 << 4) | (0x1 << 6));
}

uint8_t iic1_read_sda(void)
{
    return (GPIOB->IDR & (1 << 9)) ? 1 : 0;
}

SIAS i2c1_bus = {
    .Soft_IIC_GPIO_Port_Init = iic1_pins_config,
    .delay_us = delay_us,  // 用户需自行实现延时函数并赋值
    .dealy_ms = delay_ms,  // 用户需自行实现延时函数并赋值
    .Soft_IIC_SCL = iic1_scl,
    .Soft_IIC_SDA = iic1_sda,
    .Soft_SDA_IN = iic1_sda_in,
    .Soft_SDA_OUT = iic1_sda_out,
    .Soft_READ_SDA = iic1_read_sda,
};

#include <df_lcd.h>
#include <sh1106.h>
void SH1106_SetPixel(uint16_t x, uint16_t y, uint32_t color){
    (void)color; // 未使用参数防止编译警告
    SH1106_DrawPoint(x, y);
}
