#include "driver.h"
#include "df_led.h"

extern Lt led ;

int led_init(dev_arg_t arg) {
    (void)arg;  // 忽略参数
    // 1. 使能GPIOC时钟
    RCC->APB2ENR |= (1 << 4);  // IOPCEN = 1

    // 2. 配置PC13为推挽输出50MHz
    // CRH控制PC8~PC15，每4位控制一个引脚
    // 先清零PC13对应的4位
    GPIOC->CRH &= ~(0xF << 20);  // 20~23位
    // 设置MODE13[1:0]=11(50MHz输出)，CNF13[1:0]=00(推挽输出)
    GPIOC->CRH |= (0x3 << 20);

    // 3. 初始化PC13为低电平
    GPIOC->ODR &= ~(1 << 13);
    led.LED_Init_Flag = true;
    return 0;    // 返回0表示成功
}

int led_on(dev_arg_t arg) {
    (void)arg;  // 忽略参数
    // 设置PC11为低电平
    GPIOC->ODR &= ~(1 << 13);
    led.LED_State = true;
    return 0;  // 返回0表示成功
}

int led_off(dev_arg_t arg) {
    (void)arg;  // 忽略参数
    // 设置PC11为高电平
    GPIOC->ODR |= (1 << 13);
    led.LED_State = false;
    return 0;  // 返回0表示成功
}

int led_toggle(dev_arg_t arg) {
    (void)arg;  // 忽略参数
    // 切换PC11的状态
    GPIOC->ODR ^= (1 << 13);
    led.LED_State = !led.LED_State;
    return 0;  // 返回0表示成功
}

/* LED设备注册*/
Lt led = {
    .LED_Init_Flag = false,
    .LED_Num = 1,
    .LED_State = false,
    .LED_Name = ONBOARD_LED_NAME,
    .init = led_init,
    .on = led_on,
    .off = led_off,
    .toggle = led_toggle
};