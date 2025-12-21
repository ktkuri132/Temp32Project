#include "driver.h"
#include "df_led.h"

extern Lt led;

/**
 * STM32F4 LED驱动
 * 默认使用 PC0 作为板载LED
 */

int led_init(dev_arg_t arg)
{
    (void)arg; // 忽略参数

    // 1. 使能GPIOC时钟
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

    // 2. 配置PC0为推挽输出
    // MODER: 01 = 通用输出模式
    GPIOC->MODER &= ~(0x3 << (0 * 2));
    GPIOC->MODER |= (0x1 << (0 * 2));

    // OTYPER: 0 = 推挽输出
    GPIOC->OTYPER &= ~(1 << 0);

    // OSPEEDR: 10 = 高速
    GPIOC->OSPEEDR &= ~(0x3 << (0 * 2));
    GPIOC->OSPEEDR |= (0x2 << (0 * 2));

    // PUPDR: 00 = 无上拉下拉
    GPIOC->PUPDR &= ~(0x3 << (0 * 2));

    // 3. 初始化PC0为低电平（LED亮）
    GPIOC->ODR &= ~(1 << 0);

    led.LED_Init_Flag = true;
    return 0;
}

int led_on(dev_arg_t arg)
{
    (void)arg;
    // PC0低电平点亮LED
    GPIOC->ODR &= ~(1 << 0);
    led.LED_State = true;
    return 0;
}

int led_off(dev_arg_t arg)
{
    (void)arg;
    // PC0高电平熄灭LED
    GPIOC->ODR |= (1 << 0);
    led.LED_State = false;
    return 0;
}

int led_toggle(dev_arg_t arg)
{
    (void)arg;
    // 切换PC0状态
    GPIOC->ODR ^= (1 << 0);
    led.LED_State = !led.LED_State;
    return 0;
}

/* LED设备注册 */
Lt led = {
    .LED_Init_Flag = false,
    .LED_Num = 1,
    .LED_State = false,
    .LED_Name = ONBOARD_LED_NAME,
    .init = led_init,
    .on = led_on,
    .off = led_off,
    .toggle = led_toggle};
